#ifndef HPP_EVENTSTORE
#define HPP_EVENTSTORE

#include <util/lua_deps.h>
#include <ecs/ecs.h>

struct Lua_Functions {
    int size;
    int *functions;
};

class ECSEventStore {
public:
    ECSEventStore(const ECSEventStore&) = delete;

    static ECSEventStore& GetInstance();
    static void Initialize(lua_State *L);
    static void ECSEvent(enum ECSEvent event);

    static const char* EventToString(int event);

    int GetLuaTable();

    int m_tableId;
    lua_State *m_Lua;
    struct Lua_Functions **m_functionList;

protected:
    //void ECSEvent(lua_State *L, enum ECSEvent event, Entity e);
private:
    ECSEventStore();
    static ECSEventStore s_Instance;
    //int m_tableId;
};

#endif // HPP_EVENTSTORE
