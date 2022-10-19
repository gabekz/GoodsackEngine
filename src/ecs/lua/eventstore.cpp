#include "eventstore.hpp"
#include "../ecs.h" // definitions

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <util/lua_deps.h>
#include <lua/debug.h>

/*
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
*/

// Forward declaration
ECSEventStore::ECSEventStore() {};
ECSEventStore ECSEventStore::s_Instance;

ECSEventStore& ECSEventStore::GetInstance() { return s_Instance; }

void ECSEventStore::Initialize(lua_State *L) {
    // create function store
    lua_newtable(L);
    //dumpstack(L, "new");

    s_Instance.m_functionList = (struct Lua_Functions**)malloc(
        sizeof(struct Lua_Functions*) * ECSEVENT_LAST+1);


    for(int i = 0; i < ECSEVENT_LAST+1; i++) {
        // create a table for each event
        lua_pushstring(L, ECSEventStore::EventToString(ECSEVENT_FIRST+i));
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

void ECSEventStore::ECSEvent(enum ECSEvent event) {

    ECSEventStore &store = ECSEventStore::GetInstance();
    lua_State *L = store.m_Lua;

    const char *fName = ECSEventStore::EventToString(event);
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

int ECSEventStore::GetLuaTable() {
    ECSEventStore &store = ECSEventStore::GetInstance();
    return lua_rawgeti(
        store.m_Lua, LUA_REGISTRYINDEX,
        ECSEventStore::GetInstance().m_tableId);
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
