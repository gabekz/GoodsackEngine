#ifndef H_DEBUG
#define H_DEBUG

#include <util/lua_deps.h>

#ifdef __cplusplus
extern "C" {
#endif

int CheckLua(lua_State *L, int r);
void dumpstack (lua_State *L, const char *message);

#ifdef __cplusplus
}
#endif

#endif // H_DEBUG
