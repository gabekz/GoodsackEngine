/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "reg_system.hpp"

#include "util/logger.h"

#include "entity/ecsdefs.h"
#include "entity/lua/eventstore.hpp"
#include "wrapper/lua/lua_debug.h"

using namespace entity;

#define CHECK_REQUIRED_COMPONENTS 0

int
entity::Lua_ECSRegisterSystem(lua_State *L)
{
#if CHECK_REQUIRED_COMPONENTS
    LOG_DEBUG("Fired Register System");

    dumpstack(L, "dump from register");
    lua_getfield(L, -1, "required_components"); // get requiredComponents
    dumpstack(L, "dump from register2");

    int requiredCount = 0;
    if (!lua_isnil(L, -1))
    {
        requiredCount = lua_rawlen(L, -1);
        LOG_INFO("Required components! Obj len: %d", requiredCount);
    }

    for (int i = 0; i < requiredCount; i++)
    {
        lua_getfield(L, -1, "Camera");
        LOG_INFO("%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
    dumpstack(L, "dump from register3");

    /*
    if (lua_istable(L, -1)) {
        LOG_DEBUG("It's a table!");
        dumpstack(L, "dump from register3");
        lua_pop(L, 1);
    }
    */
#endif

    LuaEventStore &store = LuaEventStore::GetInstance();

    // retrieve table for functions
    store.RetrieveLuaTable();

    for (int i = 0; i < ECSEVENT_LAST + 1; i++)
    {
        const char *fName = ECSEVENT_STRING(i);
        lua_getfield(L, -1, fName);
        // <args>, register-table, table
        lua_getfield(L, -3, fName); // get function from <args>
        // <args>, register-table, table, function

        // TODO: check args from lua
        if (lua_isfunction(L, -1))
        {
            int f = luaL_ref(L, -2); // register to table "start"
            LuaEventStore::Lua_Functions **fList = store.getFunctionList();

            // Resize the function list
            fList[i]->functions = (int *)realloc(
              fList[i]->functions, ++fList[i]->size * sizeof(int));
            fList[i]->functions[(fList[i]->size) - 1] = f;

            LOG_INFO("function list: %s\tnewSize %i",
                     _gsk_ecs_EventToString(i),
                     fList[i]->size);
        } else
        {
            // we want to pop this if not a function. Registering as
            // reference will pop this already.
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
        // <args>, register-table
    }
    // lua_pop(L, 3);

    return 1;
}
