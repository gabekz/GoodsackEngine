#include "lua_init.hpp"

#include <stdlib.h>
#include <string.h>

#include <util/logger.h>
#include <util/lua_deps.h>

#include <ecs/lua/eventstore.hpp>
#include <ecs/lua/reg_system.hpp>

#include <wrapper/lua/lua_debug.h>
#include <wrapper/lua/lua_reg_print.h>

void
LuaInit(const char *file)
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_luaprintlib(L);

    ecs::LuaEventStore::Initialize(L);

    // add the global C function (register system) to lua
    lua_register(L, "_ECS_RegisterSystem", Lua_ECSRegisterSystem);

    if (CheckLua(L, luaL_dofile(L, file))) {

        // entry from lua [function main()]
        lua_getglobal(L, "main");
        if (lua_isfunction(L, -1)) { CheckLua(L, lua_pcall(L, 0, 0, 0)); }

        ecs::LuaEventStore::ECSEvent(ECS_INIT);
        ecs::LuaEventStore::ECSEvent(ECS_UPDATE);
        dumpstack(L, "end");
    }

    lua_close(L);
}
