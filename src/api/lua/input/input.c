#include "input.h"

#include <core/device/device.h>

#include <util/gfx.h>
#include <util/lua_deps.h>

#define MESSAGE "Hello, from input.c!"

#define MOUSE_BUTTON_INDEX_BEGIN \
    400 // Keycode for mouse buttons begin at this index

GLFWwindow *s_window;

extern int
get_key_down(lua_State *L)
{
    int keyCode = luaL_checkinteger(L, -1);

    // Handle Mouse Events
    if (keyCode >= MOUSE_BUTTON_INDEX_BEGIN) {
        lua_pushboolean(
          L,
          glfwGetMouseButton(s_window, keyCode - MOUSE_BUTTON_INDEX_BEGIN) ==
            GLFW_PRESS);

        return 1;
    }

    // Handle Keyboard Events
    lua_pushboolean(L, glfwGetKey(s_window, keyCode) == GLFW_PRESS);

    return 1;
}

extern int
get_cursor_axis(lua_State *L)
{
    // open table
    lua_newtable(L);

    lua_pushnumber(L, 1);
    lua_pushnumber(L, device_getInput().cursor_axis_raw[0]);
    lua_rawset(L, -3); // insert cell and pop

    lua_pushnumber(L, 2);
    lua_pushnumber(L, device_getInput().cursor_axis_raw[1]);
    lua_rawset(L, -3); // insert cell and pop

    lua_pushliteral(L, "n");
    lua_pushnumber(L, 2); // number of cells
    lua_rawset(L, -3);

    return 1;
}

extern int
get_cursor_state(lua_State *L)
{
    // open table
    lua_newtable(L);

    lua_pushstring(L, "is_locked");
    lua_pushboolean(L, device_getInput().cursor_state.is_locked);
    lua_rawset(L, -3); // insert cell and pop

    lua_pushstring(L, "is_visible");
    lua_pushnumber(L, device_getInput().cursor_state.is_visible);
    lua_rawset(L, -3); // insert cell and pop

    lua_pushliteral(L, "n");
    lua_pushnumber(L, 2); // number of cells
    lua_rawset(L, -3);

    return 1;
}

static const struct luaL_Reg inputFuncs[] = {
  {"GetKeyDown", get_key_down},
  {"get_cursor_axis", get_cursor_axis},
  {"get_cursor_state", get_cursor_state},
  {NULL, NULL}};

int
luaopen_hello(lua_State *L, GLFWwindow *window)
{
    s_window = window;
    // luaL_newlib(L, functions);
    // lua_register(L, "get_key_down", get_key_down);

    lua_newtable(L);
    lua_setglobal(L, "Input");

    lua_getglobal(L, "Input");
    luaL_setfuncs(L, inputFuncs, 0);
    lua_pop(L, 1);

    return 1;
}
