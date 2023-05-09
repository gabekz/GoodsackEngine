#ifndef H_API_INPUT
#define H_API_INPUT

#include <util/gfx.h>
#include <util/lua_deps.h>


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern int
say_hello(lua_State *L);

int
luaopen_hello(lua_State *L, GLFWwindow *window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_API_INPUT