/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "console_cmd.h"

#include "util/logger.h"
#include "util/sysdefs.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GSK_CONSOLE_MAX_COMMANDS     128
#define GSK_CONSOLE_MAX_ARGS         32
#define GSK_CONSOLE_MAX_INPUT_LENGTH 1024

typedef enum gsk_ConsoleCommandKind {
    GSK_CONSOLE_COMMAND_NATIVE,
    GSK_CONSOLE_COMMAND_LUA
} gsk_ConsoleCommandKind;

typedef struct gsk_ConsoleCommandDef
{
    const char *name;
    const char *usage;
    gsk_ConsoleCommandKind kind;
    union {

        gsk_Fn_ConsoleCommand native_fn;
        struct
        {
            lua_State *L;
            s32 registry_ref;
        } lua_fn;
    } cb;
} gsk_ConsoleCommandDef;

static gsk_ConsoleCommandDef s_commands[GSK_CONSOLE_MAX_COMMANDS];
static u32 s_command_count = 0;

/*--------------------------------------------------------------------*/
static u8
_tokenize_input(char *buffer, u32 *out_argc, const char **out_argv);

static s32
_find_command_index(const char *cmd_name);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void
gsk_console_cmd_register(const char *cmd_name,
                         const char *usage,
                         gsk_Fn_ConsoleCommand callback)
{
    if (cmd_name == NULL || callback == NULL)
    {
        LOG_ERROR("failed to register command: invalid args");
        return;
    }

    if (s_command_count >= GSK_CONSOLE_MAX_COMMANDS)
    {
        LOG_ERROR("command registry full");
        return;
    }

    if (_find_command_index(cmd_name) >= 0)
    {
        LOG_ERROR("duplicate command registration: %s", cmd_name);
        return;
    }

    s_commands[s_command_count].name         = cmd_name;
    s_commands[s_command_count].usage        = usage;
    s_commands[s_command_count].kind         = GSK_CONSOLE_COMMAND_NATIVE;
    s_commands[s_command_count].cb.native_fn = callback;
    s_command_count++;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
u8
gsk_console_cmd_register_lua(const char *cmd_name,
                             const char *usage,
                             lua_State *L,
                             int lua_registry_ref)
{
    if (cmd_name == NULL || L == NULL) { return FALSE; }

    // TODO: add checks here

    s32 cmd_index = _find_command_index(cmd_name);
    if (cmd_index >= 0)
    {
        LOG_WARN("duplicate command registration: %s", cmd_name);
    } else
    {
        cmd_index = s_command_count;
        s_command_count += 1;
    }

    gsk_ConsoleCommandDef *cmd = &s_commands[cmd_index];

#if 0
    if (cmd->kind != GSK_CONSOLE_COMMAND_LUA)
    {
        LOG_CRITICAL(
          "something went wrong here. probably when trying to reload.");
    }
#endif

    cmd->name                   = strdup(cmd_name);
    cmd->usage                  = usage ? strdup(usage) : NULL;
    cmd->kind                   = GSK_CONSOLE_COMMAND_LUA;
    cmd->cb.lua_fn.L            = L;
    cmd->cb.lua_fn.registry_ref = lua_registry_ref;

    return TRUE;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
u8
gsk_console_cmd_execute(const char *input_line)
{
    char buffer[GSK_CONSOLE_MAX_INPUT_LENGTH];
    const char *argv[GSK_CONSOLE_MAX_ARGS];
    u32 argc = 0;

    if (input_line == NULL) { return FALSE; }

    size_t input_len = strlen(input_line);
    if (input_len == 0) { return FALSE; }

    if (input_len >= sizeof(buffer))
    {
        LOG_ERROR("[console] input too long (max %d chars)",
                  GSK_CONSOLE_MAX_INPUT_LENGTH - 1);
        return FALSE;
    }

    memcpy(buffer, input_line, input_len + 1);

    if (!_tokenize_input(buffer, &argc, argv))
    {
        LOG_ERROR("[console] failed to parse input\n");
        return FALSE;
    }

    // call command

    const char *cmd_name = argv[0];
    s32 cmd_index        = _find_command_index(cmd_name);

    if (cmd_index < 0)
    {
        LOG_ERROR("unknown command: %s", cmd_name);
        return FALSE;
    }

    gsk_ConsoleCommandDef *cmd = &s_commands[cmd_index];

    // execute native command
    if (s_commands[cmd_index].kind == GSK_CONSOLE_COMMAND_NATIVE)
    {
        // Call callback with args AFTER the command name
        s_commands[cmd_index].cb.native_fn(argc - 1, &argv[1]);

        return TRUE;
    }

    // execute lua command

    lua_State *L = cmd->cb.lua_fn.L;
    s32 ref      = cmd->cb.lua_fn.registry_ref;

    if (L == NULL)
    {
        LOG_ERROR("lua state is null");
        return FALSE;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, ref); // push function

    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 1);
        LOG_ERROR("lua registry ref is not a function");
        return FALSE;
    }

    // TODO: look into if we should keep advancing by one argument to not pass
    // the cmd name
    for (u32 i = 1; i < argc; i++)
    {
        // TODO: check value type
        lua_pushstring(L, argv[i]);
    }

    // floor the argument count
    s32 push_c = (s32)argc - 1;
    if (push_c < 0) { push_c = 0; }

    if (lua_pcall(L, push_c, 0, 0) != LUA_OK)
    {
        const char *err = lua_tostring(L, -1);
        LOG_ERROR("lua command error: %s", err ? err : "(unknown)");
        lua_pop(L, 1);
        return FALSE;
    }

    return TRUE;
}
/*--------------------------------------------------------------------*/

/**********************************************************************/
/*   Helper Functions                                                 */
/**********************************************************************/

/*--------------------------------------------------------------------*/
static u8
_tokenize_input(char *buffer, u32 *out_argc, const char **out_argv)
{
    char *p;
    u32 argc = 0;

    if (buffer == NULL || out_argc == NULL || out_argv == NULL)
    {
        return FALSE;
    }

    p = buffer;

    while (*p != '\0')
    {
        // Skip leading whitespace
        while (*p != '\0' && isspace((unsigned char)*p))
        {
            p++;
        }

        if (*p == '\0') { break; }

        if (argc >= GSK_CONSOLE_MAX_ARGS)
        {
            LOG_ERROR("[console] too many args (max %d)", GSK_CONSOLE_MAX_ARGS);
            return FALSE;
        }

        if (*p == '"')
        {
            p++; // skip opening quote

            out_argv[argc++] = p;

            while (*p != '\0' && *p != '"')
            {
                p++;
            }

            if (*p == '"')
            {
                *p = '\0';
                p++;
            } else
            {
                LOG_ERROR("[console] unmatched quote in input");
                return FALSE;
            }
        } else
        {
            // Normal token
            out_argv[argc++] = p;

            while (*p != '\0' && !isspace((unsigned char)*p))
            {
                p++;
            }

            if (*p != '\0')
            {
                *p = '\0';
                p++;
            }
        }
    }

    *out_argc = argc;
    return TRUE;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static s32
_find_command_index(const char *cmd_name)
{
    if (cmd_name == NULL) { return -1; }

    for (u32 i = 0; i < s_command_count; i++)
    {
        if (strcmp(s_commands[i].name, cmd_name) == 0) { return (s32)i; }
    }

    return -1;
}
/*--------------------------------------------------------------------*/