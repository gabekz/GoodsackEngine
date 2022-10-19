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
    ECSEventStore(lua_State *L);
    ~ECSEventStore();

    void ECSEvent(lua_State *L, enum ECSEvent event);
    int Lua_ECSRegisterSystem(lua_State *L);
    static ECSEventStore GetInstance();
    int m_tableId;
    static const char* EventToString(int event);
protected:
    //void ECSEvent(lua_State *L, enum ECSEvent event, Entity e);
private:
    static ECSEventStore s_Instance;
    //int m_tableId;
    struct Lua_Functions **m_functionList;
};

#endif // HPP_EVENTSTORE
