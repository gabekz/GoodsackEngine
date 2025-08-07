/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LUA_HOOK_H__
#define __LUA_HOOK_H__

#include "util/lua_deps.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LUA_HOOK_RUN(EVENT, FMT, ...) \
    gsk_lua_hook_run((EVENT), (FMT), __VA_ARGS__)

#ifdef __cplusplus
}
#endif // __cplusplus

int
gsk_lua_hook_run(const char *event_name, const char *fmt, ...);

#endif // __LUA_HOOK_H__