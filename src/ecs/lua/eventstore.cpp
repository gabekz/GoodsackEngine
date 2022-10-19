#include "eventstore.hpp"
#include "../ecs.h" // definitions

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <util/lua_deps.h>
#include <lua/debug.h>

ECSEventStore::ECSEventStore(lua_State *L) {
    // create function store
    lua_newtable(L);
    //dumpstack(L, "new");

    m_functionList = (struct Lua_Functions**)malloc(
        sizeof(struct Lua_Functions*) * ECSEVENT_LAST+1);


    for(int i = 0; i < ECSEVENT_LAST+1; i++) {
        // create a table for each event
        lua_pushstring(L, ECSEventStore::EventToString(ECSEVENT_FIRST+i));
        dumpstack(L, "settable pushstring");
        lua_newtable(L);
        lua_settable(L, -3);

        // empty malloc
        m_functionList[i] = (struct Lua_Functions*)malloc(sizeof(struct Lua_Functions));
        m_functionList[i]->size = 0;

    }

    m_tableId = luaL_ref(L, LUA_REGISTRYINDEX);
    //dumpstack(L, "settable tableId");
}

ECSEventStore::~ECSEventStore() {
    // TODO: cleanup
}

int ECSEventStore::Lua_ECSRegisterSystem(lua_State *L) {
    lua_rawgeti(L,LUA_REGISTRYINDEX, m_tableId); // retrieve table for functions

    for(int i = 0; i < ECSEVENT_LAST+1; i++) {
        const char *fName = ECSEventStore::EventToString(i);
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

void ECSEventStore::ECSEvent(lua_State *L, enum ECSEvent event) {
    const char *fName = ECSEventStore::EventToString(event);
    lua_rawgeti(L,LUA_REGISTRYINDEX, m_tableId); // retrieve function table

    lua_getfield(L, -1, fName); // retrieve all functions of 'fName'
    for(int i = 0; i < m_functionList[event]->size; i++) {
        // retreive function
        lua_rawgeti(L, -1, m_functionList[event]->functions[i]);
        if(lua_isfunction(L, -1)) {
            // send data to function
            lua_pushnumber(L, 12);
            // call event function
            (CheckLua(L, lua_pcall(L, 1, 0, 0)));
        }
    }
    lua_pop(L, 2);
    // <empty>
}

const char* ECSEventStore::EventToString(int event) {
    switch(event) {
        case ECS_INIT:      return "start";
        case ECS_DESTROY:   return "destroy";
        case ECS_RENDER:    return "render";
        case ECS_UPDATE:    return "update";
        default:            return "";
    }
}
