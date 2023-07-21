#include "common.h"
#define TIME_LIB "GoodsackAPI.Time"

#include <core/device/device.h>

static int
time_delta(lua_State *L)
{
    lua_pushnumber(L, device_getAnalytics().delta);
    return 1;
}

static const luaL_Reg time_methods[] = {{"get_delta_time", time_delta},
                                        {NULL, NULL}};

int
luaopen_GoodsackAPI_Time(lua_State *L)
{
    /* create metatable */
    luaL_newmetatable(L, TIME_LIB);

    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");

    /* register methods */
    luaL_setfuncs(L, time_methods, 0);

    return 1;
}
