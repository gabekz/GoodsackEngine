#include "lua_init.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

int CheckLua(lua_State *L, int r) {
    if(r != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        printf("[Lua Error]: %s", err);
        return 0;
    }
    return 1;
}

void dumpstack(lua_State *L, const char *message) {
  printf("\n\n-------------------------\n");
  printf("[Lua] Stack Dump Begin (%s)\n", message);
  int top = lua_gettop(L);
  int s = -1;
  for (int i=top; i >= 1; i--) {
    printf("%d\t%s\t", s, luaL_typename(L,i));
    switch (lua_type(L, i)) {
      case LUA_TNUMBER:
        printf("%g\n",lua_tonumber(L,i));
        break;
      case LUA_TSTRING:
        printf("%s\n",lua_tostring(L,i));
        break;
      case LUA_TBOOLEAN:
        printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
        break;
      case LUA_TNIL:
        printf("%s\n", "nil");
        break;
      default:
        printf("%p\n",lua_topointer(L,i));
        break;
    }
    s--;
  }
  printf("\n[Lua] Stack Dump End\n");
  printf("-------------------------\n\n");
}

static int lua_print(lua_State* L) {
    int nargs = lua_gettop(L);

    for (int i=1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
            /* Pop the next arg using lua_tostring(L, i) and do your print */
            printf("\033[1;33m");
            printf("[Lua] (print): %s\n", lua_tostring(L, -1));
            printf("\033[0m");

        }
        else {
        /* Do something with non-strings if you like */
        }
    }

    return 0;
}

static const struct luaL_Reg printlib [] = {
  {"print", lua_print},
  {NULL, NULL} /* end of array */
};

extern int luaopen_luaprintlib(lua_State *L) {
  lua_getglobal(L, "_G");
  luaL_setfuncs(L, printlib, 0);
  lua_pop(L, 1);
  return 1;
}
