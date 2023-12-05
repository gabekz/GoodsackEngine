/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __REG_PRINT_H__
#define __REG_PRINT_H__

#include "util/lua_deps.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int
luaopen_luaprintlib(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // __REG_PRINT_H__
