#ifndef H_LUA_DEBUG
#define H_LUA_DEBUG

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h" 

int CheckLua(lua_State *L, int r);
void dumpstack (lua_State *L, const char *message);

extern int luaopen_luaprintlib(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // H_LUA_DEBUG
