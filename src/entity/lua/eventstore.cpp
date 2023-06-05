#include "eventstore.hpp"

#include <iostream>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/lua_deps.h>
#include <wrapper/lua/lua_debug.h>

#include <entity/component/ecs_component.hpp>
#include <entity/component/ecs_component_layout_loader.hpp>

#include <entity/v1/ecs.h>

// #include <entity/v1/builtin/transform/transform.h>

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
LuaEventStore::Initialize(lua_State *L, ECS *ecs)
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

    s_Instance.m_Layouts =
      entity::component::parse_components_from_json("../res/components.json");

    // TODO: More testing
    s_Instance.m_ecs = ecs;
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
    if (lua_isuserdata(L, 1)) {
        c = (entity::ECSComponent *)lua_topointer(L, 1);
    }

    const char *k = luaL_checkstring(L, -2);
    float var;

    if (c->GetVariable(k, &var)) {
        var = luaL_checknumber(L, -1);
        c->SetVariable(k, &var);
        return 0;
    } else {
        return luaL_argerror(
          L, -2, lua_pushfstring(L, "component does not contain '%s'", k));
    }
}

// NOTE: Return value is number of args pushed to stack
int
_meta_Component_index(lua_State *L)
{
    entity::ECSComponent *c;
    if (lua_isuserdata(L, 1)) {
        c = (entity::ECSComponent *)lua_topointer(L, 1);
    }

    const char *k = luaL_checkstring(L, -1);

    // get variable type
    vec3 vec = GLM_VEC3_ONE_INIT;
    if (c->GetVariableType(k) == EcsDataType::VEC3) {
        // LOG_DEBUG("We have a vec3");

        c->GetVariable(k, &vec);

        // open table
        lua_newtable(L);

        // create cell
        lua_pushstring(L, "x");
        lua_pushnumber(L, (float)vec[0]);
        //lua_pushnumber(L, 3);
        lua_rawset(L, -3); // insert cell and pop

        lua_pushstring(L, "y");
        lua_pushnumber(L, (float)vec[1]);
        //lua_pushnumber(L, 2);
        lua_rawset(L, -3);

        lua_pushstring(L, "z");
        lua_pushnumber(L, (float)vec[2]);
        //lua_pushnumber(L, 1);
        lua_rawset(L, -3);

        // close table
        lua_pushliteral(L, "n");
        lua_pushnumber(L, 3); // number of cells
        lua_rawset(L, -3);
        return 1; // return table
    };

    float var = 0;
    if (c->GetVariable(k, &var)) {
        lua_pushnumber(L, var);
        return 1;
    } else {
        lua_pushnil(L);
        return 0;
    }
}

static void
__create_table_for_entity_component(lua_State *L,
                                    const char *componentName,
                                    ECSComponentType componentType,
                                    Entity entity)
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

void
pushEntity(lua_State *L, int entityId, ECSComponentLayout &layout)
{

    Entity entityCompare = {.id    = (EntityId)entityId,
                            .index = (ui64)entityId,
                            .ecs   = LuaEventStore::GetInstance().m_ecs};

    std::string a = std::to_string(entityId);

    // Create "entity" as container-table
    lua_newtable(L);
    LUA_DUMP("newtable");
    lua_pushstring(L, "id");
    lua_pushnumber(L, entityId);
    LUA_DUMP("pushstring and pushnumber");
    lua_settable(L, -3);
    LUA_DUMP("settable -3");

    // TODO: Move create to separate function (per Component?)
    // use luaL_getmetatable(L, const char *tname)

    if (ecs_has(entityCompare, C_CAMERA)) {
        __create_table_for_entity_component(
          L, "Camera", C_CAMERA, entityCompare);
    }
    if (ecs_has(entityCompare, C_TRANSFORM)) {
        __create_table_for_entity_component(
          L, "Transform", C_TRANSFORM, entityCompare);
    }
    if (ecs_has(entityCompare, C_TEST)) {
        __create_table_for_entity_component(L, "Test", C_TEST, entityCompare);
    }
    if (ecs_has(entityCompare, C_WEAPON)) {
        __create_table_for_entity_component(
          L, "Weapon", C_WEAPON, entityCompare);
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

    lua_pushlightuserdata(L,
                          LuaEventStore::GetInstance()
                            .m_componentsList[C_TEST]
                            ->m_components[entityId]);
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
    for (int i = 0; i < store.m_functionList[event]->size; i++) {
        // retreive function
        LUA_DUMP("Before rawgeti"); /* stack: <[-1] Table> <[-2] Table> */
        // lua_pop(L, 1);
        lua_rawgeti(L, -1, store.m_functionList[event]->functions[i]);
        if (lua_isfunction(L, -1)) {
            // send data to function
            for (ui32 j = 0; j < store.m_ecs->nextIndex; j++) {
                // Push entity onto stack
                LUA_DUMP("dump");
                // lua_pop(L, 1);
                // lua_rawgeti(L, (int)j - 1,
                // store.m_functionList[event]->functions[i]);
                pushEntity(L, (int)j, LuaEventStore::getLayout("Camera"));
                LUA_DUMP("Pushed Entity (Table)");
                //  call event function
                (CheckLua(L, lua_pcall(L, 1, 0, 0)));
                LUA_DUMP("lua_pcall");
                // push same function for next entity
                if (j + 1 < store.m_ecs->nextIndex) {
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
