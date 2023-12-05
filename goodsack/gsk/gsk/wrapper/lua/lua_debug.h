/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LUA_DEBUG_H__
#define __LUA_DEBUG_H__

#include "util/lua_deps.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_DEBUG_ENABLE 0

#if LUA_DEBUG_ENABLE == 1
#define LUA_DUMP(message) dumpstack(L, message)
#else
#define LUA_DUMP(message) void()
#endif

int
CheckLua(lua_State *L, int r);
void
dumpstack(lua_State *L, const char *message);

#ifdef __cplusplus
}
#endif

#endif // __LUA_DEBUG_H__
