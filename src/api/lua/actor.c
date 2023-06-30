#include "common.h"

#define ACTOR_LIB "GoodsackAPI.Actor"

typedef struct Actor {
    struct Script *script;
} Actor;

/*
 * Allocate, initialize, and push a new Actor onto the stack.
 * Returns a pointer to that Actor.
 */
Actor*
lua_newactor (lua_State *L)
{
    Actor *actor = lua_malloc(L, sizeof(Actor));
    actor->script = NULL;
    return actor;
}

/*
 * Make sure the argument at index N is a actor and return it if it is.
 */
Actor*
lua_checkactor (lua_State *L, int index)
{
    return (Actor *) luaL_checkudata(L, index, ACTOR_LIB);
}

static int
actor_new (lua_State* L)
{
    Actor *actor = lua_newactor(L);
    lua_pushobject(L, actor, ACTOR_LIB);
    return 1;
}

static int
actor_print (lua_State* L)
{
    Actor *actor = lua_checkactor(L, 1);
    lua_pushfstring(L, "%s %p", ACTOR_LIB, actor);
    return 1;
}

static const luaL_Reg actor_methods[] = {
    {"__tostring", actor_print},
    { NULL, NULL }
};

int 
luaopen_GoodsackAPI_Actor (lua_State * L)
{
    /* create metatable */
    luaL_newmetatable(L, ACTOR_LIB);

    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");

    /* register methods */
    luaL_setfuncs(L, actor_methods, 0);

    /* Actor() => new Actor */
    lua_pushcfunction(L, actor_new);

    return 1;
}
