/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "lua_reg_print.h"

#include "util/ansi_colors.h"
#include "util/logger.h"
#include "wrapper/lua/lua_debug.h"

static int
lua_print(lua_State *L)
{
    int nargs = lua_gettop(L);

    for (int i = 1; i <= nargs; i++)
    {
        LOG_PRINT(BYEL "[Lua] (print): %s" COLOR_RESET,
                  (lua_isstring(L, i) ? lua_tostring(L, -1)
                                      : lua_tolstring(L, -1, NULL)));
    }

    return 0;
}

static const struct luaL_Reg printlib[] = {
  {"print", lua_print}, {NULL, NULL} /* end of array */
};

extern int
luaopen_luaprintlib(lua_State *L)
{
    lua_getglobal(L, "_G");
    luaL_setfuncs(L, printlib, 0);
    lua_pop(L, 1);
    return 1;
}
