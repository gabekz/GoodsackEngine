/*
#include "ecs_lua.h"
#include "ecs.h" // definitions

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

const char* EventToString(enum ECSEvent event) {
    switch(event) {
        case ECS_INIT:      return "init";
        case ECS_DESTROY:   return "destroy";
        case ECS_RENDER:    return "render";
        case ECS_UPDATE:    return "update";
    }
}

ECSEventStore::ECSEventStore(lua_State *L) {
    m_tableId = 0;
    // create function store
    lua_newtable(L);
    //dumpstack(L, "new");

    m_functionList = (struct Lua_Functions**)malloc(
        sizeof(struct Lua_Functions*) * ECSEVENT_LAST+1);


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
*/
