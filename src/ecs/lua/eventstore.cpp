#include "eventstore.hpp"

#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <util/lua_deps.h>
#include <lua/debug.h>

#include <ecs/ecs.h>
#include <ecs/component.hpp>

#include <components/transform/transform.h>

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

int _meta_Component_index(lua_State *L) {
    return -1;
    
    // 1. Grab the entity from args
    // 2. use the ID for checkudata to reference the saved pointer

    //Component *c = (Component *)luaL_checkudata(L, 1, "ComponentTransform");

}

int _lua_Transform_index(lua_State *L) {
    struct ComponentTransform *t = (struct ComponentTransform*)luaL_checkudata(L, 1, "ComponentTransform");
    //dumpstack(L, "index");
    const char *k = luaL_checkstring(L, -1);
    //dumpstack(L, "index");
    if(!strcmp(k, "test")) {
        lua_pushnumber(L, t->test);
    }
    else if(!strcmp(k, "subset")) {
        //lua_rawgeti(L, -1, 1);

        //luaL_checktype(L, -1, LUA_TTABLE);
        //luaL_getmetatable(L, "MyStruct");
        //lua_getfield(L, -1, "subset");
        //luaL_getsubtable(L, -1, "subset");
        //luaL_getmetafield(L, 1, "health");
        //
        //iterate_and_print(L, -1);

        //dumpstack(L, "note: subset gettable");
    }
    else {
        lua_pushnil(L);
    }
    return 1;
}

// Creates a Lua entity with metatables
static void pushEntity(lua_State *L) {

    // test structure
    struct ComponentTransform *t = (struct ComponentTransform*)malloc(sizeof(struct ComponentTransform));
    t->test = 80;

    // Create "entity" as container-table
    lua_newtable(L);
    lua_pushstring(L, "id");
    lua_pushnumber(L, 19);
    lua_settable(L, -3);

    lua_pushstring(L, "ComponentTransform"); // temp

    // Create new metatable
    luaL_newmetatable(L, "ComponentTransform");
    lua_pushcfunction(L, _lua_Transform_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    lua_pushlightuserdata(L, t);
    luaL_setmetatable(L, "ComponentTransform");
    lua_settable(L, -3);
}

/*
// Creates a Lua entity with metatables
static void pushEntity2(lua_State *L) {

    t->test = 80;

    // Create "entity" as container-table
    lua_newtable(L);
    lua_pushstring(L, "id");
    lua_pushnumber(L, 19);
    lua_settable(L, -3);

    lua_pushstring(L, "ComponentTransform"); // temp

    // Create new metatable
    luaL_newmetatable(L, "ComponentTransform");
    lua_pushcfunction(L, _lua_Transform_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    lua_pushlightuserdata(L, t);
    luaL_setmetatable(L, "ComponentTransform");
    lua_settable(L, -3);

}
*/

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
            pushEntity(L);
            //lua_pushnumber(L, 12);
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
