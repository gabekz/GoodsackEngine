#include "common.h"

#include "GoodsackEngineConfig.h"

#define GOODSACK_LIB "goodsack"

static int
goodsack_version_info(lua_State *L)
{
    lua_pushfstring(L,
                    "%s %d.%d.%d.%d",
                    GOODSACK_LIB,
                    GOODSACK_VERSION_MAJOR,
                    GOODSACK_VERSION_MINOR,
                    GOODSACK_VERSION_PATCH,
                    GOODSACK_VERSION_TWEAK);
    return 1;
}

static const luaL_Reg goodsack_methods[] = {
  {"__tostring", goodsack_version_info},
  {"version_info", goodsack_version_info},
  {NULL, NULL}};

int
luaopen_goodsack(lua_State *L)
{
    /* create metatable */
    luaL_newmetatable(L, GOODSACK_LIB);

    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");

    /* register methods */
    luaL_setfuncs(L, goodsack_methods, 0);

    return 1;
}
