#ifndef HPP_EVENTSTORE
#define HPP_EVENTSTORE

#include <entity/component/ecs_component.hpp>
#include <entity/component/ecs_component_layout_loader.hpp>
#include <util/lua_deps.h>

#include <entity/ecs_entity.hpp>
#include <entity/ecsdefs.h>

#include <map>
#include <string>

#include <entity/v1/ecs.h>

namespace entity {

class LuaEventStore {
   public:
    LuaEventStore(const LuaEventStore &) = delete;

    static LuaEventStore &GetInstance();
    static void Initialize(lua_State *L, ECS *ecs);
    static void ECSEvent(enum ECSEvent event);

    static ECSComponentLayout &getLayout(const char *layout)
    {
        return *LuaEventStore::GetInstance().m_Layouts[layout];
    };

    // TEST
    // static Entity entity;

    struct Lua_Functions
    {
        int size;
        int *functions;
    };

    int RetrieveLuaTable();
    struct Lua_Functions **getFunctionList() { return m_functionList; };

    // TESTING
    ECSComponent **m_componentsList;
    size_t m_componentsListCount;

    // TESTING 2
    ECS *m_ecs;

   protected:
    struct Lua_Functions **m_functionList;

   private:
    LuaEventStore();
    static LuaEventStore s_Instance;
    std::map<std::string, ECSComponentLayout *> m_Layouts;

    int m_tableId;
    lua_State *m_Lua;

    // TODO: remove after testing
    ECSEntity **m_entityList;
    size_t m_entityListCount;
};

} // namespace entity

#endif // HPP_EVENTSTORE
