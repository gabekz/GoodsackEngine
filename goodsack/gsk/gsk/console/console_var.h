/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_CONSOLE_VAR_H__
#define __GSK_CONSOLE_VAR_H__

#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum GskConsoleVarType_ {
    GskConsoleVarType_Integer = 0,
    GskConsoleVarType_Float,
    GskConsoleVarType_String,
    GskConsoleVarType_Bool,
} GskConsoleVarType_;
typedef u8 GskConsoleVarType;

void
gsk_console_var_register_int(const char *name,
                             const char *description,
                             s32 default_value,
                             s32 min_value,
                             s32 max_value);

void
gsk_console_var_register_float(const char *name,
                               const char *description,
                               f32 default_value,
                               f32 min_value,
                               f32 max_value);

void
gsk_console_var_register_string(const char *name,
                                const char *description,
                                const char *default_value);

u8
gsk_console_var_set_from_string(const char *name, const char *value_str);

s32
gsk_console_var_get_int(const char *name, s32 fallback);

f32
gsk_console_var_get_float(const char *name, f32 fallback);

const char *
gsk_console_var_get_string(const char *name, const char *fallback);

u8
gsk_console_var_print(const char *name);

#if 0
void
gsk_console_var_register_float(const char *name,
                               const char *description,
                               f32 default_value,
                               f32 min_value,
                               f32 max_value);

void
gsk_console_var_register_string(const char *name,
                                const char *description,
                                const char *default_value);

void
gsk_console_var_register_bool(const char *name,
                              const char *description,
                              u8 default_value);

void
gsk_console_var_register(const char *var_name);

void
gsk_console_var_get(const char *var_name);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__GSK_CONSOLE_CMD_H__
