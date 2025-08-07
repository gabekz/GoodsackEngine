/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LUA_LIBS_H__
#define __LUA_LIBS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_LIB
#include "util/lua_deps.h"

void
open_all_libs(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // __LUA_LIBS_H__