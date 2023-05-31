#ifndef H_LUA_LIBS
#define H_LUA_LIBS

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_LIB
#include <util/lua_deps.h>

void
open_all_libs(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // H_LUA_LIBS