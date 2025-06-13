/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "lua_debug.h"

#include "util/ansi_colors.h"
#include "util/logger.h"

int
CheckLua(lua_State *L, int r)
{
    if (r == LUA_OK) { return 1; }

    const char *err = lua_tostring(L, -1);
    LOG_PRINT(BRED "[Lua] (error): %s" COLOR_RESET, err);
    return 0;
}

void
dumpstack(lua_State *L, const char *message)
{
    printf("\n\n-------------------------\n");
    printf("[Lua] Stack Dump Begin (%s)\n", message);
    int top = lua_gettop(L);
    int s   = -1;
    for (int i = top; i >= 1; i--)
    {
        printf("%d\t%s\t", s, luaL_typename(L, i));
        switch (lua_type(L, i))
        {
        case LUA_TNUMBER: printf("%g\n", lua_tonumber(L, i)); break;
        case LUA_TSTRING: printf("%s\n", lua_tostring(L, i)); break;
        case LUA_TBOOLEAN:
            printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL: printf("%s\n", "nil"); break;
        default: printf("%p\n", lua_topointer(L, i)); break;
        }
        s--;
    }
    printf("\n[Lua] Stack Dump End\n");
    printf("-------------------------\n\n");
}
