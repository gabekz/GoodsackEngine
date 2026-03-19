/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_CONSOLE_CMD_H__
#define __GSK_CONSOLE_CMD_H__

#include "util/sysdefs.h"
#include "wrapper/lua/lua_libs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if 0
typedef enum gsk_ConsoleArgType {
    GSK_CONSOLE_ARG_STRING,
    GSK_CONSOLE_ARG_INT,
    GSK_CONSOLE_ARG_FLOAT,
    GSK_CONSOLE_ARG_BOOL
} gsk_ConsoleArgType;

typedef struct gsk_ConsoleArg
{
    gsk_ConsoleArgType type;
    union {
        const char *s;
        s32 i;
        f32 f;
        u8 b;
    } as;
} gsk_ConsoleArg;
#endif

typedef void (*gsk_Fn_ConsoleCommand)(u32 argc, const char **argv);

void
gsk_console_cmd_register(const char *cmd_name,
                         const char *usage,
                         gsk_Fn_ConsoleCommand callback);

#if 0
void
gsk_console_cmd_execute(const char *cmd_name, const char *param);
#endif

u8
gsk_console_cmd_execute(const char *input_line);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__GSK_CONSOLE_CMD_H__
