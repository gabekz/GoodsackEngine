#include "common.h"

#define GOODSACK_LIB "GoodsackAPI"

static int
goodsack_print (lua_State* L)
{
    lua_pushfstring(L, "%s %d", GOODSACK_LIB, 3);
    return 1;
}

static int
goodsack_test(lua_State* L)
{
    printf("oh shit, BOY!");
    return 0;
}

static const luaL_Reg goodsack_methods[] = {
    {"__tostring", goodsack_print},
    {"testfunc", goodsack_test},
    { NULL, NULL }
};

int 
luaopen_GoodsackAPI(lua_State * L)
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
