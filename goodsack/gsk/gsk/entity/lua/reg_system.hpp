/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __REG_SYSTEM_HPP__
#define __REG_SYSTEM_HPP__

#include "util/lua_deps.h"

namespace entity {

extern "C" {
int
Lua_ECSRegisterSystem(lua_State *L);
}
} // namespace entity

#endif // __REG_SYSTEM_HPP__