/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "lua_init.hpp"

#include <stdlib.h>
#include <string.h>

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/lua_deps.h"

#include "entity/lua/eventstore.hpp"
#include "entity/lua/reg_system.hpp"
#include "entity/ecs.h"

#include "wrapper/lua/lua_debug.h"
#include "wrapper/lua/lua_reg_print.h"

void
LuaInit(const char *file, gsk_ECS *ecs)
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    luaopen_luaprintlib(L);

    entity::LuaEventStore::Initialize(L, ecs);

    // add the global C function (register system) to lua
    lua_register(L, "_ECS_RegisterSystem", entity::Lua_ECSRegisterSystem);

    if (!CheckLua(L, luaL_dofile(L, GSK_PATH(LUA_INIT_FILE_PATH)))) {
        LOG_CRITICAL("Failed to read engine lua entry: %s",
                     GSK_PATH(LUA_INIT_FILE_PATH));
    }

    if (!CheckLua(L, luaL_dofile(L, file))) {
        LOG_CRITICAL("Failed to read lua entry: %s", file);
    }

    // lua_close(L);
}
