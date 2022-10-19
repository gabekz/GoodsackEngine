#include "eventstore.hpp"
#include "../ecs.h" // definitions

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <util/lua_deps.h>
#include <lua/debug.h>

using namespace ecs;

// Forward declaration
LuaEventStore::LuaEventStore() {};
LuaEventStore LuaEventStore::s_Instance;

LuaEventStore& LuaEventStore::GetInstance() { return s_Instance; }

void LuaEventStore::Initialize(lua_State *L) {
    // create function store
    lua_newtable(L);
    //dumpstack(L, "new");

    s_Instance.m_functionList = (struct Lua_Functions**)malloc(
        sizeof(struct Lua_Functions*) * ECSEVENT_LAST+1);


    for(int i = 0; i < ECSEVENT_LAST+1; i++) {
        // create a table for each event
        lua_pushstring(L, LuaEventStore::EventToString(ECSEVENT_FIRST+i));
        dumpstack(L, "settable pushstring");
        lua_newtable(L);
        lua_settable(L, -3);

        // empty malloc
        s_Instance.m_functionList[i] = (struct Lua_Functions*)malloc(sizeof(struct Lua_Functions*));
        s_Instance.m_functionList[i]->size = 0;
        s_Instance.m_functionList[i]->functions = (int *)calloc(0, sizeof(int));

    }

    s_Instance.m_tableId = luaL_ref(L, LUA_REGISTRYINDEX);
    s_Instance.m_Lua = L;
    //dumpstack(L, "settable tableId");
}

void LuaEventStore::ECSEvent(enum ECSEvent event) {

    LuaEventStore &store = LuaEventStore::GetInstance();
    lua_State *L = store.m_Lua;

    const char *fName = LuaEventStore::EventToString(event);
    lua_rawgeti(L, LUA_REGISTRYINDEX, store.m_tableId); // retrieve function table

    lua_getfield(L, -1, fName); // retrieve all functions of 'fName'
    for(int i = 0; i < store.m_functionList[event]->size; i++) {
        // retreive function
        lua_rawgeti(L, -1, store.m_functionList[event]->functions[i]);
        dumpstack(L, "getfunctionfromlist");
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

int LuaEventStore::RetrieveLuaTable() {
    LuaEventStore &store = LuaEventStore::GetInstance();
    return lua_rawgeti(
        store.m_Lua, LUA_REGISTRYINDEX,
        LuaEventStore::GetInstance().m_tableId);
}

const char* LuaEventStore::EventToString(int event) {
    switch(event) {
        case ECS_INIT:      return "start";
        case ECS_DESTROY:   return "destroy";
        case ECS_RENDER:    return "render";
        case ECS_UPDATE:    return "update";
        default:            return "";
    }
}
