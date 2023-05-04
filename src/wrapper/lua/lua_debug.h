#ifndef H_DEBUG
#define H_DEBUG

#include <util/lua_deps.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_DEBUG_ENABLE 0

#if LUA_DEBUG_ENABLE == 1
#define LUA_DUMP(message) dumpstack(L, message)
#else
#define LUA_DUMP(message) void()
#endif

int
CheckLua(lua_State *L, int r);
void
dumpstack(lua_State *L, const char *message);

#ifdef __cplusplus
}
#endif

#endif // H_DEBUG
