#include "common.h"

#include "GoodsackEngineConfig.h"

#define GOODSACK_LIB            "goodsack"
#define LLIB_GOODSACK_IS_GLOBAL 1

static int
_VERSION(lua_State *L)
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
  {"__tostring", _VERSION}, {"VERSION", _VERSION}, {NULL, NULL}};

int
luaopen_goodsack(lua_State *L)
{
    /* create metatable */
#if LLIB_GOODSACK_IS_GLOBAL
    lua_getglobal(L, "_G");
#else
    luaL_newmetatable(L, GOODSACK_LIB);
    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
#endif // LLIB_GOODSACK_IS_GLOBAL

    /* register methods */
    luaL_setfuncs(L, goodsack_methods, 0);

#if LLIB_GOODSACK_IS_GLOBAL
    lua_pop(L, 1);
#endif // LLIB_GOODSACK_IS_GLOBAL

    return 1;
}
