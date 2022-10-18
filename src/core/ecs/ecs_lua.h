#include "../ecs.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

struct Lua_Functions {
    int size;
    int *functions;
};

#ifdef __cplusplus
}

class ECSEventStore {
public:
    ECSEventStore(lua_State *L);
    ~ECSEventStore();
protected:
    int _lua_ECSRegisterSystem(lua_State *L);
    void _lua_ECSEvent(lua_State *L, enum ECSEvent event, Entity e);
private:
    int m_tableId;
    struct Lua_Functions **functionList;
};

#endif // C++ only

struct TLua_ECSEventStore {
    int tableId;
    struct Lua_Functions **functionList;
};

struct TLua_ECSEventStore* Tcreate_ecs_eventstore(lua_State *L);
void ecs_lua_event(
        lua_State *L, struct TLua_ECSEventStore *store,
        enum ECSEvent event, Entity e);


