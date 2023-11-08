#include "lua_reg_print.h"

#include <util/logger.h>
#include <wrapper/lua/lua_debug.h>

static int
lua_print(lua_State *L)
{
    int nargs = lua_gettop(L);

    for (int i = 1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
            /* Pop the next arg using lua_tostring(L, i) and do your print */
            LOG_PRINT("\033[1;33m");
            LOG_PRINT("[Lua] (print): %s", lua_tostring(L, -1));
            LOG_PRINT("\033[0m");
        } else {
            LOG_PRINT("\033[1;33m");
            LOG_PRINT("[Lua] (print): %s", luaL_tolstring(L, -1, NULL));
            LOG_PRINT("\033[0m");
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
