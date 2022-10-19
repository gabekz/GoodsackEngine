#ifndef H_ECS_LUA
#define H_ECS_LUA

#include <ecs/ecs.h>
#include <util/lua_deps.h>

/*
struct TLua_Functions {
    int size;
    int *functions;
};
*/

struct TLua_ECSEventStore {
    int tableId;
    struct Lua_Functions **functionList;
};

struct TLua_ECSEventStore* Tcreate_ecs_eventstore(lua_State *L);
void ecs_lua_event(
        lua_State *L, struct TLua_ECSEventStore *store,
        enum ECSEvent event, Entity e);

#endif // H_ECS_LUA
