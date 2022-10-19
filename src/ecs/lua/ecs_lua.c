#include "ecs_lua.h"
#include <ecs/ecs.h>

#include <stdlib.h>
#include <stdio.h>

static const char* EventToString(enum ECSEvent event) {
    switch(event) {
        case ECS_INIT:      return "init";
        case ECS_DESTROY:   return "destroy";
        case ECS_RENDER:    return "render";
        case ECS_UPDATE:    return "update";
    }
}

struct TLua_ECSEventStore* Tcreate_ecs_eventstore(lua_State *L) {
    struct TLua_ECSEventStore *store = 
        malloc(sizeof(struct TLua_ECSEventStore));

    // create function store
    lua_newtable(L);
    //dumpstack(L, "new");

    struct Lua_Functions **functions;
    functions = malloc(sizeof(struct Lua_Functions*) * ECSEVENT_LAST+1);


    for(int i = 0; i < ECSEVENT_LAST+1; i++) {
        // create a table for each event
        lua_pushstring(L, EventToString(ECSEVENT_FIRST+i));
        lua_newtable(L);
        lua_settable(L, -3);

        // empty malloc
        functions[i] = malloc(sizeof(struct Lua_Functions));
        functions[i]->size = 0;

    //dumpstack(L, "settable start");
    }

    store->tableId = luaL_ref(L, LUA_REGISTRYINDEX);
    store->functionList = functions;
    //dumpstack(L, "settable tableId");

    return store;
}

// TODO: impl
int _lua_ecs_register_system(lua_State *L) {
    //lua_rawgeti(L,LUA_REGISTRYINDEX, store->tableId); // retrieve table for functions

    for(int i = 0; i < ECSEVENT_LAST+1; i++) {
        const char *fName = EventToString(i);
        lua_getfield(L, -1, fName);
        // <args>, register-table, table
        lua_getfield(L, -3, fName); // get function from <args>
        // <args>, register-table, table, function

        // TODO: check args from lua
        if(lua_isfunction(L, -1)) {
            int f = luaL_ref(L, -2); // register to table "start"
            // TODO: Add ECS Event Store (add_ecs_eventstore)
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

void ecs_lua_event(lua_State *L, struct TLua_ECSEventStore *store,
        enum ECSEvent event, Entity entity) {

    const char *fName = EventToString(event);
    lua_rawgeti(L,LUA_REGISTRYINDEX, store->tableId); // retrieve function table

    lua_getfield(L, -1, fName); // retrieve all functions of 'fName'
    for(int i = 0; i < store->functionList[event]->size; i++) {
        // retreive function
        lua_rawgeti(L, -1, store->functionList[event]->functions[i]);
        if(lua_isfunction(L, -1)) {
            // send data to function
            lua_pushnumber(L, 12);
            // call event function
            //     (CheckLua(L, lua_pcall(L, 1, 0, 0)));
        }
    }
    lua_pop(L, 2);
    // <empty>
}
