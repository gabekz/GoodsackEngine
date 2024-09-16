/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "../common.h"
#define INPUT_LIB "goodsack.input"

#include "util/gfx.h"

#include "core/device/device.h"
#include "runtime/gsk_runtime.hpp"

extern "C" {

#define MOUSE_BUTTON_INDEX_BEGIN \
    400 // Keycode for mouse buttons begin at this index

extern int
_GetKeyDown(lua_State *L)
{
    int keycode          = luaL_checkinteger(L, -1);
    GLFWwindow *p_window = gsk_runtime_get_renderer()->window;

    // Handle Mouse Events
    if (keycode >= MOUSE_BUTTON_INDEX_BEGIN)
    {
        lua_pushboolean(
          L,
          glfwGetMouseButton(p_window, keycode - MOUSE_BUTTON_INDEX_BEGIN) ==
            GLFW_PRESS);

        return 1;
    }

    // Handle Keyboard Events
    lua_pushboolean(L, glfwGetKey(p_window, keycode) == GLFW_PRESS);

    return 1;
}

extern int
_GetCursorAxis(lua_State *L)
{
    // open table
    lua_newtable(L);

    lua_pushnumber(L, 1);
    lua_pushnumber(L, gsk_device_getInput().cursor_axis_raw[0]);
    lua_rawset(L, -3); // insert cell and pop

    lua_pushnumber(L, 2);
    lua_pushnumber(L, gsk_device_getInput().cursor_axis_raw[1]);
    lua_rawset(L, -3); // insert cell and pop

    lua_pushliteral(L, "n");
    lua_pushnumber(L, 2); // number of cells
    lua_rawset(L, -3);

    return 1;
}

extern int
_GetCursorState(lua_State *L)
{
    // open table
    lua_newtable(L);

    lua_pushstring(L, "is_locked");
    lua_pushboolean(L, gsk_device_getInput().cursor_state.is_locked);
    lua_rawset(L, -3); // insert cell and pop

    lua_pushstring(L, "is_visible");
    lua_pushnumber(L, gsk_device_getInput().cursor_state.is_visible);
    lua_rawset(L, -3); // insert cell and pop

    lua_pushliteral(L, "n");
    lua_pushnumber(L, 2); // number of cells
    lua_rawset(L, -3);

    return 1;
}

static const luaL_Reg lib_input_methods[] = {
  {"GetKeyDown", _GetKeyDown},
  {"GetCursorAxis", _GetCursorAxis},
  {"GetCursorState", _GetCursorState},
  {NULL, NULL}};

extern int
luaopen_goodsack_input(lua_State *L)
{
    /* create metatable */
    luaL_newmetatable(L, INPUT_LIB);

    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");

    /* register methods */
    luaL_setfuncs(L, lib_input_methods, 0);

    return 1;
}
}