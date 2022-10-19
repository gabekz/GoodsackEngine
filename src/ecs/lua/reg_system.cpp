#include "reg_system.hpp"

#include <ecs/lua/eventstore.hpp>

int Lua_ECSRegisterSystem(lua_State *L) {
    ecs::LuaEventStore &store = ecs::LuaEventStore::GetInstance();

    // retrieve table for functions
    store.RetrieveLuaTable();

    for(int i = 0; i < ECSEVENT_LAST+1; i++) {
        const char *fName = ecs::LuaEventStore::EventToString(i);
        lua_getfield(L, -1, fName);
        // <args>, register-table, table
        lua_getfield(L, -3, fName); // get function from <args>
        // <args>, register-table, table, function

        // TODO: check args from lua
        if(lua_isfunction(L, -1)) {
            int f = luaL_ref(L, -2); // register to table "start"
            // TODO: Add ECS Event Store (add_ecs_eventstore)
            //int s = ++store.m_functionList[i]->size;
            ecs::LuaEventStore::Lua_Functions **fList = store.getFunctionList();
            fList[i]->functions = (int*)realloc(
                fList[i]->functions, ++fList[i]->size * sizeof(int));
            fList[i]->functions[(fList[i]->size)-1] = f;
        }
        else {
            // we want to pop this if not a function. Registering as
            // reference will pop this already.
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
        // <args>, register-table
    }
    //lua_pop(L, 3);

    return 1;
}
