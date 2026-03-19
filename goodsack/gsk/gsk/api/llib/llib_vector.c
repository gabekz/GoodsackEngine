/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "common.h"

#include "llib_vector.h"

#include <stdio.h>
#include <stdlib.h>

#include "util/logger.h"
#include "util/maths.h"

static int
_vector__OPERATOR(lua_State *L, int lua_operation)
{
    gsk_L_Vector *vec_a = luaL_checkudata(L, 1, VECTOR_LIB);

    gsk_L_Vector *vec_b = NULL;
    f32 b_number        = 0;

    gsk_L_Vector *ret = NULL;

    u8 is_b_scalar = (!lua_isuserdata(L, 2));

    // grab number
    if (is_b_scalar)
    {
        b_number = luaL_checknumber(L, 2);
    }
    // grab vector
    else
    {
        vec_b = luaL_checkudata(L, 2, VECTOR_LIB);
    }

    ret = lua_newuserdata(L, sizeof(gsk_L_Vector));

    // Vector *ret = malloc(sizeof(Vector));
    // if (ret == NULL) { LOG_CRITICAL("Failed to allocate lua Vector"); }

    if (is_b_scalar)
    {
        switch (lua_operation)
        {
        case (LUA_OPADD):
            glm_vec3_adds(vec_a->float3, b_number, ret->float3);
            break;
        case (LUA_OPSUB):
            glm_vec3_subs(vec_a->float3, b_number, ret->float3);
            break;
        default: LOG_ERROR("Unknown Operation");
        }
    } else
    {
        switch (lua_operation)
        {
        case (LUA_OPADD):
            glm_vec3_add(vec_a->float3, vec_b->float3, ret->float3);
            break;
        case (LUA_OPSUB):
            glm_vec3_sub(vec_a->float3, vec_b->float3, ret->float3);
            break;
        default: LOG_ERROR("Unknown Operation");
        }
    }

    //*(Vector **)lua_newuserdata(L, sizeof(Vector *)) = ret;
    luaL_setmetatable(L, VECTOR_LIB);

    return 1;
}

static int
vector__gc(lua_State *L)
{
    LOG_TRACE("## __gc\n");
    // Vector *foo = *(Vector **)luaL_checkudata(L, 1, VECTOR_LIB);
    // free(foo);
    return 0;
}

static int
vector__tostring(lua_State *L)
{
    gsk_L_Vector *foo = luaL_checkudata(L, 1, VECTOR_LIB);
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

static int
vector_Cross(lua_State *L)
{
    LOG_DEBUG("## Cross\n");
    gsk_L_Vector *vec_a = luaL_checkudata(L, 1, VECTOR_LIB);
    gsk_L_Vector *vec_b = luaL_checkudata(L, 2, VECTOR_LIB);

    // Vector *ret = malloc(sizeof(Vector));
    gsk_L_Vector *ret = lua_newuserdata(L, sizeof(gsk_L_Vector));
    // if (ret == NULL) { LOG_CRITICAL("Failed to allocate lua Vector"); }

    glm_vec3_cross(vec_a->float3, vec_b->float3, ret->float3);

    luaL_setmetatable(L, VECTOR_LIB);

    return 1;
}

static int
vector_new(lua_State *L)
{
    LOG_TRACE("## new\n");

    // Vector *new_vec = malloc(sizeof(Vector));
    // if (new_vec == NULL) { LOG_CRITICAL("Failed to allocate lua Vector"); }
    gsk_L_Vector *new_vec = lua_newuserdata(L, sizeof(gsk_L_Vector));

    int iter = 1 + lua_istable(L, 1);

    for (int i = 0; i < 3 /*vec3 values*/; i++)
    {
        int j = iter + i;
        new_vec->float3[i] =
          !lua_isnoneornil(L, j) ? luaL_checknumber(L, j) : 0;
    }

    //*(Vector **)lua_newuserdata(L, sizeof(Vector *)) = new_vec;
    luaL_setmetatable(L, VECTOR_LIB);
    return 1;
}

static int
vector__index(lua_State *L)
{
    gsk_L_Vector *vec = luaL_checkudata(L, 1, VECTOR_LIB);
    const char *key   = luaL_checkstring(L, 2);

    if (strcmp(key, "x") == 0)
    {
        lua_pushnumber(L, vec->float3[0]);
        return 1;
    }
    if (strcmp(key, "y") == 0)
    {
        lua_pushnumber(L, vec->float3[1]);
        return 1;
    }
    if (strcmp(key, "z") == 0)
    {
        lua_pushnumber(L, vec->float3[2]);
        return 1;
    }

    /* fallback to methods stored in __methods */
    luaL_getmetatable(L, VECTOR_LIB);
    lua_getfield(L, -1, "__methods");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);

    return 1;
}

static int
vector__newindex(lua_State *L)
{
    gsk_L_Vector *vec = luaL_checkudata(L, 1, VECTOR_LIB);
    const char *key   = luaL_checkstring(L, 2);
    float value       = luaL_checknumber(L, 3);

    LOG_INFO("vec3 __newindex %f", value);

    if (strcmp(key, "x") == 0)
    {
        vec->float3[0] = value;
        return 0;
    }
    if (strcmp(key, "y") == 0)
    {
        vec->float3[1] = value;
        return 0;
    }
    if (strcmp(key, "z") == 0)
    {
        vec->float3[2] = value;
        return 0;
    }

    return luaL_error(L, "invalid Vector field '%s'", key);
}

int
luaopen_goodsack_vector(lua_State *L)
{
    // instance functions
    static const luaL_Reg meta[] = {{"__gc", vector__gc},
                                    {"__tostring", vector__tostring},
                                    {"__add", vector__add},
                                    {"__sub", vector__sub},
                                    {"__index", vector__index},
                                    {"__newindex", vector__newindex},
                                    {NULL, NULL}};

    static const luaL_Reg meth[] = {{"Cross", vector_Cross}, {NULL, NULL}};

    luaL_newmetatable(L, VECTOR_LIB); // -- metatable: goodsack.vector
    luaL_setfuncs(L, meta, 0);
    luaL_newlib(L, meth);
    lua_setfield(L, -2, "__methods");

    lua_pop(L, 1);

    // static functions -- Vector

    static const luaL_Reg static_meta[] = {{"__call", vector_new},
                                           {NULL, NULL}};
    static const luaL_Reg static_meth[] = {{"new", vector_new}, {NULL, NULL}};
    luaL_newlib(L, static_meth);
    luaL_newlib(L, static_meta);
    lua_setmetatable(L, -2);
    return 1;
}
