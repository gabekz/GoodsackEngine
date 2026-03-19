/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "common.h"

#include "gsk_generated/GoodsackEngineConfig.h"

#include "console/console_cmd.h"
#include "util/logger.h"

#define GOODSACK_LIB            "goodsack"
#define LLIB_GOODSACK_IS_GLOBAL 1

static int
_VERSION(lua_State *L)
{
    lua_pushfstring(L,
                    "%s %d.%d.%d.%d",
                    GOODSACK_LIB,
                    GOODSACK_VERSION_MAJOR,
                    GOODSACK_VERSION_MINOR,
                    GOODSACK_VERSION_PATCH,
                    GOODSACK_VERSION_TWEAK);
    return 1;
}

static int
_RunConsoleCommand(lua_State *L)
{
#if 0
    const char *input_line = luaL_checkstring(L, 1);

    u8 result = gsk_console_cmd_execute(input_line);
    lua_pushboolean(L, result);
    return 1;
#endif
    int top = lua_gettop(L);

    if (top < 1)
    {
        return luaL_error(L, "RunConsoleCommand requires at least 1 arg");
    }

    /*
     * If exactly one string arg, treat it as full command line.
     */
    if (top == 1)
    {
        const char *input_line = luaL_checkstring(L, 1);
        lua_pushboolean(L, gsk_console_cmd_execute(input_line));
        return 1;
    }

    /*
     * Otherwise build a command line from multiple Lua args.
     */
    luaL_Buffer b;
    luaL_buffinit(L, &b);

    for (int i = 1; i <= top; i++)
    {
        size_t len;
        const char *s = luaL_tolstring(L, i, &len);

        if (i > 1) { luaL_addchar(&b, ' '); }

        /*
         * Quote args containing whitespace or quotes.
         * Very simple implementation.
         */
        u8 needs_quotes = FALSE;
        for (size_t j = 0; j < len; j++)
        {
            if (isspace((unsigned char)s[j]) || s[j] == '"')
            {
                needs_quotes = TRUE;
                break;
            }
        }

        if (needs_quotes)
        {
            luaL_addchar(&b, '"');
            for (size_t j = 0; j < len; j++)
            {
                if (s[j] == '"')
                {
                    /*
                     * This is weak because your parser does not yet support
                     * escapes. So either reject quotes here or improve parser.
                     */
                    lua_pop(L, 1);
                    return luaL_error(L,
                                      "quotes inside args not supported yet");
                }
                luaL_addchar(&b, s[j]);
            }
            luaL_addchar(&b, '"');
        } else
        {
            luaL_addlstring(&b, s, len);
        }

        lua_pop(L, 1); /* pop luaL_tolstring result */
    }

    luaL_pushresult(&b);
    const char *input_line = lua_tostring(L, -1);

    u8 ok = gsk_console_cmd_execute(input_line);
    lua_pop(L, 1);

    lua_pushboolean(L, ok);
    return 1;
}

static int
_CreateConsoleCommand(lua_State *L)
{
    const char *cmd_name = luaL_checkstring(L, 1);

    const char *usage = NULL;
    int fn_index      = 0;

    if (lua_isfunction(L, 2))
    {
        fn_index = 2;
    } else
    {
        usage = luaL_checkstring(L, 2);
        luaL_checktype(L, 3, LUA_TFUNCTION);
        fn_index = 3;
    }

    /*
     * Copy function into registry
     */
    lua_pushvalue(L, fn_index);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    u8 ok = gsk_console_cmd_register_lua(cmd_name, usage, L, ref);
    if (!ok)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        lua_pushboolean(L, 0);
        lua_pushfstring(L, "failed to register console command '%s'", cmd_name);
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}

static const luaL_Reg goodsack_methods[] = {
  {"__tostring", _VERSION},
  {"VERSION", _VERSION},
  {"RunConsoleCommand", _RunConsoleCommand},
  {"CreateConsoleCommand", _CreateConsoleCommand},
  {NULL, NULL}};

int
luaopen_goodsack(lua_State *L)
{
    /* create metatable */
#if LLIB_GOODSACK_IS_GLOBAL
    lua_getglobal(L, "_G");
#else
    luaL_newmetatable(L, GOODSACK_LIB);
    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
#endif // LLIB_GOODSACK_IS_GLOBAL

    /* register methods */
    luaL_setfuncs(L, goodsack_methods, 0);

#if LLIB_GOODSACK_IS_GLOBAL
    lua_pop(L, 1);
#endif // LLIB_GOODSACK_IS_GLOBAL

    return 1;
}
