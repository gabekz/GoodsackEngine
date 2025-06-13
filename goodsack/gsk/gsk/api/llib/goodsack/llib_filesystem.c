/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "../common.h"
#define FILESYSTEM_LIB "goodsack.filesystem"

#include "util/filesystem.h"

static int
path_from_uri(lua_State *L)
{
    const char *str_uri = luaL_checkstring(L, -1);
    lua_pushstring(L, GSK_PATH(str_uri));
    return 1;
}

static const luaL_Reg lib_fs_methods[] = {{"URI", path_from_uri}, {NULL, NULL}};

int
luaopen_goodsack_filesystem(lua_State *L)
{
    /* create metatable */
    luaL_newmetatable(L, FILESYSTEM_LIB);

    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");

    /* register methods */
    luaL_setfuncs(L, lib_fs_methods, 0);

    return 1;
}