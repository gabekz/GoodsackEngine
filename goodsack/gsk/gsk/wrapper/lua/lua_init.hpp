/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LUA_INIT_H__
#define __LUA_INIT_H__

#include "entity/ecs.h"
#include "util/sysdefs.h"

// TODO: replace hardcoded path
#define LUA_INIT_FILE_PATH "gsk://../gsk/gsk/api/lua/init.lua"

void
LuaInit(const char *file, gsk_ECS *ecs);

#endif // __LUA_INIT_H__
