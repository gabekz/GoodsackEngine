#ifndef COMMON_H
#define COMMON_H

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/*
 * Allocates size_t memory on the given Lua state.
 * lua_newuserdata automatically pushes it, so we pop it.
 */
static
void *
lua_malloc (lua_State *L, size_t size)
{
    void *p = lua_newuserdata(L, size);
    //lua_pop(L, 1);
    return p;
}

/*
 * Associate an object pointer with given name and push that onto the stack.
 */
static
void
lua_pushobject (lua_State *L, void *object_pointer, const char *metatable_name)
{
    //lua_pushlightuserdata(L, object_pointer);
    luaL_getmetatable(L, metatable_name);
    lua_setmetatable(L, -2);
}

#endif
