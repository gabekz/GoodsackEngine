#ifndef HPP_EVENTSTORE
#define HPP_EVENTSTORE

#include <util/lua_deps.h>
#include <ecs/ecs.h>

namespace ecs {


class LuaEventStore {
public:
    LuaEventStore(const LuaEventStore&) = delete;

    static LuaEventStore& GetInstance();
    static void Initialize(lua_State *L);
    static void ECSEvent(enum ECSEvent event);

    static const char* EventToString(int event);

    // TEST
    static Entity entity;

    struct Lua_Functions {
        int size;
        int *functions;
    };

    int RetrieveLuaTable();
    struct Lua_Functions **getFunctionList() { return m_functionList; };

protected:
    struct Lua_Functions **m_functionList;

private:
    LuaEventStore();
    static LuaEventStore s_Instance;

    int m_tableId;
    lua_State *m_Lua;
};

} // namespace

#endif // HPP_EVENTSTORE