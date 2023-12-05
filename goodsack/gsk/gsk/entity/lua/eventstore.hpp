/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __EVENTSTORE_HPP__
#define __EVENTSTORE_HPP__

#include <entity/component/ecs_component.hpp>
#include <entity/component/ecs_component_layout_loader.hpp>
#include <util/lua_deps.h>

#include <entity/ecsdefs.h>

#include <map>
#include <string>

#include <entity/v1/ecs.h>

namespace entity {

class LuaEventStore {
   public:
    LuaEventStore(const LuaEventStore &) = delete;

    static LuaEventStore &GetInstance();
    static void Initialize(lua_State *L, gsk_ECS *ecs);
    static void ECSEvent(enum ECSEvent event);

    static ECSComponentLayout &getLayout(const char *layout)
    {
        return *LuaEventStore::GetInstance().m_Layouts[layout];
    };

    // TEST
    // static gsk_Entity entity;

    struct Lua_Functions
    {
        int size;
        int *functions;
    };

    int RetrieveLuaTable();
    struct Lua_Functions **getFunctionList() { return m_functionList; };

    // TESTING
    // ECSComponent **m_componentsList;
    // size_t m_componentsListCount;
    ECSComponentList *m_componentsList[ECSCOMPONENT_LAST + 1];

    void RegisterComponentList(ECSComponentType componentIndex,
                               ECSComponentLayout &layout);
    void RegisterComponentList(ECSComponentType componentIndex,
                               const char *layoutKey);

    // TESTING 2
    gsk_ECS *m_ecs;

   protected:
    struct Lua_Functions **m_functionList;

   private:
    LuaEventStore();
    static LuaEventStore s_Instance;
    std::map<std::string, ECSComponentLayout *> m_Layouts;

    int m_tableId;
    lua_State *m_Lua;
};

} // namespace entity

#endif // __EVENTSTORE_HPP__
