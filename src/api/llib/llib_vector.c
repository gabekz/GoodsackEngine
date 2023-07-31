#include "common.h"
#define VECTOR_LIB "goodsack.vector"

#include <stdio.h>
#include <stdlib.h>

#include <util/logger.h>

typedef struct // Vector_lua_t
{
    int x;
} Vector;

static int
vector_gc(lua_State *L)
{
    LOG_TRACE("## __gc\n");
    Vector *foo = *(Vector **)luaL_checkudata(L, 1, VECTOR_LIB);
    free(foo);
    return 0;
}

static int
vector_DoSomething(lua_State *L)
{
    LOG_TRACE("## DoSomething\n");
    Vector *foo = *(Vector **)luaL_checkudata(L, 1, VECTOR_LIB);
    lua_pushinteger(L, foo->x * 20);
    return 1;
}

static int
vector_new(lua_State *L)
{
    LOG_TRACE("## new\n");
    Vector *foo = malloc(sizeof(Vector));
    int i       = 1 + lua_istable(L, 1);
    foo->x      = !lua_isnoneornil(L, i) ? luaL_checkinteger(L, i) : 0;
    *(Vector **)lua_newuserdata(L, sizeof(Vector *)) = foo;
    luaL_setmetatable(L, VECTOR_LIB);
    return 1;
}

static int
vector_index(lua_State *L)
{
    LOG_TRACE("## index\n");
    int i = luaL_checkinteger(L, 2);
    lua_pushinteger(L, i);
    return 1;
}

int
luaopen_goodsack_vector(lua_State *L)
{
    // instance functions
    static const luaL_Reg meta[] = {{"__gc", vector_gc}, {NULL, NULL}};
    static const luaL_Reg meth[] = {{"DoSomething", vector_DoSomething},
                                    {NULL, NULL}};
    luaL_newmetatable(L, VECTOR_LIB);
    luaL_setfuncs(L, meta, 0);
    luaL_newlib(L, meth);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    // static functions
    static const luaL_Reg static_meta[] = {
      {"__index", vector_index}, {"__call", vector_new}, {NULL, NULL}};
    static const luaL_Reg static_meth[] = {{"new", vector_new}, {NULL, NULL}};
    luaL_newlib(L, static_meth);
    luaL_newlib(L, static_meta);
    lua_setmetatable(L, -2);
    return 1;
}