/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "eventstore.hpp"

#include <iostream>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/filesystem.h"
#include "util/lua_deps.h"

#include "wrapper/lua/lua_debug.h"

#include "entity/component/ecs_component.hpp"
#include "entity/component/ecs_component_layout_loader.hpp"
#include "entity/ecs.h"

#define EVENTSTORE_LUA_VEC3_CLASS 1

using namespace entity;

// Forward declaration
LuaEventStore::LuaEventStore() {};
LuaEventStore LuaEventStore::s_Instance;

static void
pushEntity(lua_State *L, u64 entity_index);

#if EVENTSTORE_LUA_VEC3_CLASS

static void
pushVector(lua_State *L);

#endif

LuaEventStore &
LuaEventStore::GetInstance()
{
    return s_Instance;
}

void
LuaEventStore::Initialize(lua_State *L, gsk_ECS *ecs)
{
    // create function store
    lua_newtable(L);

    s_Instance.m_functionList = (struct Lua_Functions **)malloc(
      sizeof(struct Lua_Functions *) * ECSEVENT_LAST + 1);

    for (int i = 0; i < ECSEVENT_LAST + 1; i++)
    {
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

    s_Instance.m_Layouts = entity::component::parse_components_from_json(
      GSK_PATH("gsk://components.json"));

    // TODO: More testing
    s_Instance.m_ecs = ecs;
}

void
LuaEventStore::Cleanup()
{
    LOG_DEBUG("cleaning up");
    if (s_Instance.m_Lua) { lua_close(s_Instance.m_Lua); }
    // TODO: cleanup ECS related stuff
}

void
LuaEventStore::RegisterComponentList(ECSComponentType componentIndex,
                                     ECSComponentLayout &layout)
{
    entity::LuaEventStore::GetInstance().m_componentsList[componentIndex] =
      new entity::ECSComponentList(componentIndex, layout);
}

void
LuaEventStore::RegisterComponentList(ECSComponentType componentIndex,
                                     const char *layoutKey)
{
    entity::LuaEventStore::GetInstance().m_componentsList[componentIndex] =
      new entity::ECSComponentList(componentIndex,
                                   entity::LuaEventStore::getLayout(layoutKey));
}

int
_meta_Component_newindex(lua_State *L)
{
    entity::ECSComponent *c;
    if (lua_isuserdata(L, 1))
    {
        c = (entity::ECSComponent *)lua_topointer(L, 1);
    }

    const char *k = luaL_checkstring(L, -2);
    float var;

    // get variable type
    if (c->GetVariableType(k) == EcsDataType::VEC3)
    {
        vec3 vec = GLM_VEC3_ONE_INIT;

        lua_pushnil(L); // first key
        int stackIndex = -2, iter = 0;
        while (lua_next(L, stackIndex))
        { // traverse keys
            vec[iter] = lua_tonumber(L, -1);
            lua_pop(L, 1); // stack restore
            iter++;
        }

        c->SetVariable(k, &vec);

        return 0;
    }

    if (c->GetVariable(k, &var))
    {
        var = luaL_checknumber(L, -1);
        c->SetVariable(k, &var);
        return 0;
    }

    return luaL_argerror(
      L, -2, lua_pushfstring(L, "component does not contain '%s'", k));
}

// NOTE: Return value is number of args pushed to stack
int
_meta_Component_index(lua_State *L)
{
    entity::ECSComponent *c;
    if (lua_isuserdata(L, 1))
    {
        c = (entity::ECSComponent *)lua_topointer(L, 1);
    }

    const char *k = luaL_checkstring(L, -1);

    // get variable type
    if (c->GetVariableType(k) == EcsDataType::VEC3)
    {
        // LOG_DEBUG("We have a vec3");
        vec3 vec = GLM_VEC3_ONE_INIT;

        c->GetVariable(k, &vec);

        // open table
        lua_newtable(L);

        // create cell
        lua_pushstring(L, "x");
        lua_pushnumber(L, (float)vec[0]);
        // lua_pushnumber(L, 3);
        lua_rawset(L, -3); // insert cell and pop

        lua_pushstring(L, "y");
        lua_pushnumber(L, (float)vec[1]);
        // lua_pushnumber(L, 2);
        lua_rawset(L, -3);

        lua_pushstring(L, "z");
        lua_pushnumber(L, (float)vec[2]);
        // lua_pushnumber(L, 1);
        lua_rawset(L, -3);

        // close table
        lua_pushliteral(L, "n");
        lua_pushnumber(L, 3); // number of cells
        lua_rawset(L, -3);
        return 1; // return table

    } else if (c->GetVariableType(k) == EcsDataType::ENTITY)
    {
        int entityId = -1;
        c->GetVariable(k, &entityId);
        pushEntity(L, (int)entityId);
        return 1;
    }

    float var = 0;
    if (c->GetVariable(k, &var))
    {
        lua_pushnumber(L, var);
        return 1;
    } else
    {
        lua_pushnil(L);
        return 0;
    }
}

static void
__create_table_for_entity_component(lua_State *L,
                                    const char *componentName,
                                    ECSComponentType componentType,
                                    gsk_Entity entity)
{

    std::string a         = std::to_string(entity.id);
    const char *tableName = a.append(componentName).c_str();

    // Create new metatable
    lua_pushstring(L, componentName); // temp
    LUA_DUMP("pushstring");
    luaL_newmetatable(L, tableName);
    lua_pushcfunction(L, _meta_Component_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, _meta_Component_newindex);
    lua_setfield(L, -2, "__newindex");
    LUA_DUMP("After new metatable");
    lua_pop(L, 1);
    LUA_DUMP("After new metatable POP");

    lua_pushlightuserdata(L,
                          LuaEventStore::GetInstance()
                            .m_componentsList[componentType]
                            ->m_components[entity.id]);
    LUA_DUMP("After pushlightuserdata");
    luaL_setmetatable(L, tableName);
    LUA_DUMP("After setmetatable");
    lua_settable(L, -3);
    LUA_DUMP("After settable -3");
}

// TODO: handle read/write access +
static void
pushEntity(lua_State *L, u64 entity_index)
{
    gsk_ECS *p_ecs = LuaEventStore::GetInstance().m_ecs;

    gsk_EntityId entity_id = p_ecs->p_ent_ids[entity_index];

    gsk_Entity entityCompare = {.id    = p_ecs->p_ent_ids[entity_index],
                                .index = entity_index,
                                .ecs   = p_ecs};

    std::string a = std::to_string(entity_id);

    // Create "entity" as container-table
    lua_newtable(L);
    LUA_DUMP("newtable");
    lua_pushstring(L, "id");
    lua_pushnumber(L, entity_id);
    // lua_pushstring(L, "index");
    // lua_pushnumber(L, entity_index);
    LUA_DUMP("pushstring and pushnumber");
    lua_settable(L, -3);
    LUA_DUMP("settable -3");

    // TODO: Move create to separate function (per Component?)
    // use luaL_getmetatable(L, const char *tname)

    if (gsk_ecs_has(entityCompare, C_CAMERA))
    {
        __create_table_for_entity_component(
          L, "Camera", C_CAMERA, entityCompare);
    }
    if (gsk_ecs_has(entityCompare, C_CAMERA_LOOK))
    {
        __create_table_for_entity_component(
          L, "CameraLook", C_CAMERA_LOOK, entityCompare);
    }
    if (gsk_ecs_has(entityCompare, C_CAMERA_MOVEMENT))
    {
        __create_table_for_entity_component(
          L, "CameraMovement", C_CAMERA_MOVEMENT, entityCompare);
    }
    if (gsk_ecs_has(entityCompare, C_TRANSFORM))
    {
        __create_table_for_entity_component(
          L, "Transform", C_TRANSFORM, entityCompare);
    }
    if (gsk_ecs_has(entityCompare, C_WEAPON))
    {
        __create_table_for_entity_component(
          L, "Weapon", C_WEAPON, entityCompare);
    }
    if (gsk_ecs_has(entityCompare, C_WEAPON_SWAY))
    {
        __create_table_for_entity_component(
          L, "WeaponSway", C_WEAPON_SWAY, entityCompare);
    }

#if 0
    // Second metatable? //TODO: Testing

    const char *tableName2 = a.append("Test").c_str();

    // Create new metatable
    lua_pushstring(L, "Test"); // temp
    LUA_DUMP("pushstring");
    luaL_newmetatable(L, tableName2);
    lua_pushcfunction(L, _meta_Component_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, _meta_Component_newindex);
    lua_setfield(L, -2, "__newindex");
    LUA_DUMP("After new metatable");
    lua_pop(L, 1);
    LUA_DUMP("After new metatable POP");

#if 0
    lua_pushlightuserdata(L,
                          LuaEventStore::GetInstance()
                            .m_componentsList[C_TEST]
                            ->m_components[entityId]);
#endif
    LUA_DUMP("After pushlightuserdata");
    luaL_setmetatable(L, tableName2);
    LUA_DUMP("After setmetatable");
    lua_settable(L, -3);
    LUA_DUMP("After settable -3");
#endif
}

void
LuaEventStore::ECSEvent(enum ECSEvent event)
{
    LuaEventStore &store = LuaEventStore::GetInstance();
    lua_State *L         = store.m_Lua;

    /* stack: <empty> */

    const char *fName = ECSEVENT_STRING(event);
    lua_rawgeti(
      L, LUA_REGISTRYINDEX, store.m_tableId); // retrieve function table

    lua_getfield(L, -1, fName); // retrieve all functions of 'fName'
    for (int i = 0; i < store.m_functionList[event]->size; i++)
    {
        // retreive function
        LUA_DUMP("Before rawgeti"); /* stack: <[-1] Table> <[-2] Table> */
        lua_rawgeti(L, -1, store.m_functionList[event]->functions[i]);

#if 0
        if(!lua_isfunction(L, -1)) {
            continue;
        }
#endif

        if (lua_isfunction(L, -1))
        {
            // send data to function
            for (u32 j = 0; j < store.m_ecs->nextIndex; j++)
            {
                // Push entity onto stack
                LUA_DUMP("dump");

                pushEntity(L, (u64)j);
                LUA_DUMP("Pushed Entity (Table)");

                //  call event function
                bool status = CheckLua(L, lua_pcall(L, 1, 0, 0));
                LUA_DUMP("lua_pcall");

                if (status == false) { break; }

                // push same function for next entity
                if (j + 1 < store.m_ecs->nextIndex)
                {
                    lua_rawgeti(
                      L, -1, store.m_functionList[event]->functions[i]);
                    LUA_DUMP("j+1 <= nextIndex -> rawgeti -1");
                }
            }
        }
    }
    lua_pop(L, 2);
    LUA_DUMP("Should be empty...");
    // <empty>
}

int
LuaEventStore::RetrieveLuaTable()
{
    LuaEventStore &store = LuaEventStore::GetInstance();
    return lua_rawgeti(
      store.m_Lua, LUA_REGISTRYINDEX, LuaEventStore::GetInstance().m_tableId);
}
