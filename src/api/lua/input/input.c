#include "input.h"

#include <core/device/device.h>

#include <util/gfx.h>
#include <util/lua_deps.h>

#define MESSAGE "Hello, from input.c!"

GLFWwindow *s_window;

extern int
say_hello(lua_State *L)
{
    lua_pushstring(L, MESSAGE);
    return 1;
}

extern int
get_time(lua_State *L)
{
    lua_pushnumber(L, device_getAnalytics().delta);
    return 1;
}

extern int
get_key_down(lua_State *L)
{
    int keyCode = luaL_checkinteger(L, -1);
    lua_pushboolean(L, glfwGetKey(s_window, keyCode) == GLFW_PRESS);
    return 1;
}

static const struct luaL_Reg functions[] = {{"say_hello", say_hello},
                                            {NULL, NULL}};

static const struct luaL_Reg inputFuncs[] = {{"GetKeyDown", get_key_down},
                                            {NULL, NULL}};

int
luaopen_hello(lua_State *L, GLFWwindow *window)
{
    s_window = window;
    // luaL_newlib(L, functions);
    lua_register(L, "hello", say_hello);
    lua_register(L, "delta_time", get_time);
    //lua_register(L, "get_key_down", get_key_down);

    lua_newtable(L);
    lua_setglobal(L, "Input");

    lua_getglobal(L, "Input");
    luaL_setfuncs(L, inputFuncs, 0);
    lua_pop(L, 1);

    return 1;
}