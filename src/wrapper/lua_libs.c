
#define LUA_LIB
#include <wrapper/lua/lua_libs.h>
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

static const luaL_Reg lualibs[] = {{"", luaopen_base},
                                   {LUA_LOADLIBNAME, luaopen_package},
                                   {LUA_TABLIBNAME, luaopen_table},
                                   {LUA_STRLIBNAME, luaopen_string},
                                   {LUA_MATHLIBNAME, luaopen_math},
                                   {LUA_DBLIBNAME, luaopen_debug},
                                   {NULL, NULL}};

LUALIB_API void
my_openlibs(lua_State *L)
{
    const luaL_Reg *lib = lualibs;
    for (; lib->func; lib++) {
        lua_pushcfunction(L, lib->func);
        lua_pushstring(L, lib->name);
        lua_call(L, 1, 0);
    }
}

void
open_all_libs(lua_State *L)
{
    my_openlibs(L);
}