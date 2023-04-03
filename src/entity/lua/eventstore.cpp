#include "eventstore.hpp"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/lua_deps.h>
#include <wrapper/lua/lua_debug.h>

#include <entity/component/component.hpp>
#include <entity/component/layout.hpp>
#include <entity/component/loader.hpp>

#include <entity/v1/builtin/transform/transform.h>

using namespace entity;

// Forward declaration
LuaEventStore::LuaEventStore() {};
LuaEventStore LuaEventStore::s_Instance;

LuaEventStore &
LuaEventStore::GetInstance()
{
    return s_Instance;
}

void
LuaEventStore::Initialize(lua_State *L)
{
    // create function store
    lua_newtable(L);

    s_Instance.m_functionList = (struct Lua_Functions **)malloc(
      sizeof(struct Lua_Functions *) * ECSEVENT_LAST + 1);

    for (int i = 0; i < ECSEVENT_LAST + 1; i++) {
        // create a table for each event
        lua_pushstring(L, ECSEVENT_STRING(ECSEVENT_FIRST + i));
        lua_newtable(L);
        lua_settable(L, -3);

        // empty malloc
        s_Instance.m_functionList[i] =
          (struct Lua_Functions *)malloc(sizeof(struct Lua_Functions *));
        s_Instance.m_functionList[i]->size      = 0;
        s_Instance.m_functionList[i]->functions = (int *)calloc(0, sizeof(int));
    }

    s_Instance.m_tableId = luaL_ref(L, LUA_REGISTRYINDEX);
    s_Instance.m_Lua     = L;

    s_Instance.m_Layouts = entity::ParseComponents("../res/components.json");
}

int
_meta_Component_newindex(lua_State *L)
{
    Component *c;
    if (lua_isuserdata(L, 1)) { c = (Component *)lua_topointer(L, 1); }

    const char *k = luaL_checkstring(L, -2);
    int var;

    if (c->GetVariable(k, &var)) {
        var = luaL_checknumber(L, -1);
        c->SetVariable(k, &var);
        return 0;
    } else {
        return luaL_argerror(
          L, -2, lua_pushfstring(L, "component does not contain '%s'", k));
    }
}

int
_meta_Component_index(lua_State *L)
{
    Component *c;
    if (lua_isuserdata(L, 1)) { c = (Component *)lua_topointer(L, 1); }

    const char *k = luaL_checkstring(L, -1);
    int var;
    if (c->GetVariable(k, &var)) {
        lua_pushnumber(L, var);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

void
pushEntity(lua_State *L, Entity entity, ComponentLayout &layout)
{

    Component *t = new Component(layout);

    std::string a         = std::to_string(entity.id);
    const char *tableName = a.append(layout.getName()).c_str();

    // Create "entity" as container-table
    lua_newtable(L);
    lua_pushstring(L, "id");
    lua_pushnumber(L, 19);
    lua_settable(L, -3);

    lua_pushstring(L, "ComponentCamera"); // temp

    /*
    int speed = 32;
    t->SetVariable("speed", &speed);
    */

    // Create new metatable
    luaL_newmetatable(L, tableName);
    lua_pushcfunction(L, _meta_Component_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, _meta_Component_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);

    lua_pushlightuserdata(L, t);
    luaL_setmetatable(L, tableName);
    lua_settable(L, -3);
}

void
LuaEventStore::ECSEvent(enum ECSEvent event)
{

    LuaEventStore &store = LuaEventStore::GetInstance();
    lua_State *L         = store.m_Lua;

    const char *fName = ECSEVENT_STRING(event);
    lua_rawgeti(
      L, LUA_REGISTRYINDEX, store.m_tableId); // retrieve function table

    lua_getfield(L, -1, fName); // retrieve all functions of 'fName'
    for (int i = 0; i < store.m_functionList[event]->size; i++) {
        // retreive function
        lua_rawgeti(L, -1, store.m_functionList[event]->functions[i]);
        if (lua_isfunction(L, -1)) {
            // send data to function
            // pushEntity(L);
            pushEntity(L,
                       (Entity {.id = 19}),
                       LuaEventStore::getLayout("ComponentCamera"));
            // lua_pushnumber(L, 12);
            //  call event function
            (CheckLua(L, lua_pcall(L, 1, 0, 0)));
        }
    }
    lua_pop(L, 2);
    // <empty>
}

int
LuaEventStore::RetrieveLuaTable()
{
    LuaEventStore &store = LuaEventStore::GetInstance();
    return lua_rawgeti(
      store.m_Lua, LUA_REGISTRYINDEX, LuaEventStore::GetInstance().m_tableId);
}
