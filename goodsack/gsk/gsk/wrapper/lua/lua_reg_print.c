/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "lua_reg_print.h"

#include "util/ansi_colors.h"
#include "util/logger.h"
#include "wrapper/lua/lua_debug.h"

static void
_print_table(lua_State *L, int idx, int depth)
{
    // make sure idx is absolute, so pushes above don't shift it
    idx = lua_absindex(L, idx);

    // iterate table
    lua_pushnil(L);               // stack: … table … nil
    while (lua_next(L, idx) != 0) // stack: … table … key value
    {
        // indent
        for (int i = 0; i < depth; i++)
            putchar(' ');

        // key → string
        const char *key;
        if (lua_type(L, -2) == LUA_TSTRING)
            key = lua_tostring(L, -2);
        else
        {
            luaL_tolstring(L, -2, NULL); // converts key at -2 to string
            key = lua_tostring(L, -1);
            lua_pop(L, 1); // pop the temp string
        }

        // if value is a nested table, recurse…
        if (lua_istable(L, -1))
        {
            LOG_PRINT(BYEL "\t%s = {\n" COLOR_RESET, key);
            _print_table(L, -1, depth + 2);
            for (int i = 0; i < depth; i++)
                putchar(' ');
            LOG_PRINT(BYEL "\t}\n" COLOR_RESET);
        } else
        {
            // convert value → string
            luaL_tolstring(L, -1, NULL); // pushes the stringified value
            const char *val = lua_tostring(L, -1);
            LOG_PRINT(BYEL "\t%s = %s" COLOR_RESET, key, val);
            lua_pop(L, 1); // pop the temp string
        }

        lua_pop(L, 1); // pop value, keep key for next lua_next
    }
}

static int
lua_print(lua_State *L)
{
    int nargs = lua_gettop(L);

    for (int i = 1; i <= nargs; i++)
    {
        if (lua_istable(L, i))
        {
            LOG_PRINT(BYEL "[Lua] (print): table:" COLOR_RESET);
            _print_table(L, i, 0);
        } else
        {
            // everything else: coerce to string
            luaL_tolstring(L, i, NULL); // pushes stringified version
            const char *s = lua_tostring(L, -1);
            LOG_PRINT(BYEL "[Lua] (print): %s" COLOR_RESET, s);
            lua_pop(L, 1); // pop the temp string
        }
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
