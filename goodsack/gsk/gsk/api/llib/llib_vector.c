/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "common.h"
#define VECTOR_LIB "goodsack.vector"

#include <stdio.h>
#include <stdlib.h>

#include "util/logger.h"
#include "util/maths.h"

typedef struct // Vector_lua_t
{
    float float3[3];
} Vector;

#if 0
static int
vector__index(lua_State *L)
{
    //const char *key   = luaL_checkstring(L, -2);
    //const char *value = luaL_checkstring(L, -1);
    if (!lua_rawget(L, 1)) { LOG_INFO("lua raw get");
    }

    return 1;
}
#endif

static int
_vector__OPERATOR(lua_State *L, int lua_operation)
{
    Vector *vec_a = *(Vector **)luaL_checkudata(L, 1, VECTOR_LIB);
    Vector *vec_b = *(Vector **)luaL_checkudata(L, 2, VECTOR_LIB);

    Vector *ret = malloc(sizeof(Vector));

    switch (lua_operation) {
    case (LUA_OPADD): glm_vec3_add(vec_a, vec_b, ret->float3); break;
    case (LUA_OPSUB): glm_vec3_sub(vec_a, vec_b, ret->float3); break;
    default: LOG_ERROR("Unknown Operation");
    }

    *(Vector **)lua_newuserdata(L, sizeof(Vector *)) = ret;
    luaL_setmetatable(L, VECTOR_LIB);

    return 1;
}

static int
vector__gc(lua_State *L)
{
    LOG_TRACE("## __gc\n");
    Vector *foo = *(Vector **)luaL_checkudata(L, 1, VECTOR_LIB);
    free(foo);
    return 0;
}

static int
vector__tostring(lua_State *L)
{
    Vector *foo = *(Vector **)luaL_checkudata(L, 1, VECTOR_LIB);
    lua_pushfstring(
      L, "{%f, %f, %f} ", foo->float3[0], foo->float3[1], foo->float3[2]);
    return 1;
}

static int
vector__add(lua_State *L)
{
    return _vector__OPERATOR(L, LUA_OPADD);
}

static int
vector__sub(lua_State *L)
{
    return _vector__OPERATOR(L, LUA_OPSUB);
}

#if 0
static int
vector_get_x(lua_State *L)
{
    if (!CheckLua(L, luaL_testudata(L, 1, VECTOR_LIB))) {
        LOG_DEBUG("ITS NOT UDATA");
    }
    const char *key   = luaL_checkstring(L, -1);
    const char *value = luaL_checkstring(L, -1);

    LOG_INFO("Key: %s, Value: %s", key, value);

    LOG_TRACE("## get_x\n");
    Vector *foo = *(Vector **)luaL_checkudata(L, 1, VECTOR_LIB);
    lua_pushinteger(L, foo->float3[0]);
    return 1;
}
#endif

static int
vector_Cross(lua_State *L)
{
    LOG_DEBUG("## Cross\n");
    Vector *vec_a = *(Vector **)luaL_checkudata(L, 1, VECTOR_LIB);
    Vector *vec_b = *(Vector **)luaL_checkudata(L, 2, VECTOR_LIB);

    Vector *ret = malloc(sizeof(Vector));
    glm_vec3_cross(vec_a, vec_b, ret->float3);

    *(Vector **)lua_newuserdata(L, sizeof(Vector *)) = ret;
    luaL_setmetatable(L, VECTOR_LIB);

    return 1;
}

static int
vector_new(lua_State *L)
{
    LOG_TRACE("## new\n");
    Vector *foo = malloc(sizeof(Vector));

    int iter = 1 + lua_istable(L, 1);

    for (int i = 0; i < 3 /*vec3 values*/; i++) {
        int j          = iter + i;
        foo->float3[i] = !lua_isnoneornil(L, j) ? luaL_checkinteger(L, j) : 0;
    }

    *(Vector **)lua_newuserdata(L, sizeof(Vector *)) = foo;
    luaL_setmetatable(L, VECTOR_LIB);
    return 1;
}

#if 0
static int
vector_index(lua_State *L)
{
    LOG_TRACE("## index\n");

    const char *k = luaL_checkstring(L, -1);
    LOG_INFO("Checked is %s", k);

    int i = luaL_checkinteger(L, 2);
    lua_pushinteger(L, i);
    return 1;
}
#endif

int
luaopen_goodsack_vector(lua_State *L)
{
    // instance functions
    static const luaL_Reg meta[] = {{"__gc", vector__gc},
                                    {"__tostring", vector__tostring},
                                    {"__add", vector__add},
                                    {"__sub", vector__sub},
                                    //{"__index", vector__index},
                                    {NULL, NULL}};

    static const luaL_Reg meth[] = {{"Cross", vector_Cross}, {NULL, NULL}};

    luaL_newmetatable(L, VECTOR_LIB); // -- metatable: goodsack.vector
    luaL_setfuncs(L, meta, 0);
    // lua_pushcfunction(L, vector__index); //-- instead of newlib(L, meth)
    luaL_newlib(L, meth);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

// static functions -- Vector
#if 0
    static const luaL_Reg static_meta[] = {
      {"__index", vector_index}, {"__call", vector_new}, {NULL, NULL}};
#endif
    static const luaL_Reg static_meta[] = {{"__call", vector_new},
                                           {NULL, NULL}};
    static const luaL_Reg static_meth[] = {{"new", vector_new}, {NULL, NULL}};
    luaL_newlib(L, static_meth);
    luaL_newlib(L, static_meta);
    lua_setmetatable(L, -2);
    return 1;
}
