#include "common.h"

#define SCRIPT_LIB "GoodsackAPI.Actor.Script"

typedef struct Script {
    const char *string;
} Script;

/*
 * Allocate a new Script to be passed around.
 */
Script *
lua_newscript (lua_State *L, const char *string)
{
    if (string == NULL)
        luaL_error(L, "`string` cannot be empty!");

    Script *script = (Script*) lua_malloc(L, sizeof(Script));
    script->string = string;
    return script;
}

/*
 * Make sure the argument at index N is a Script and return it if it is.
 */
Script *
lua_checkscript (lua_State *L, int index)
{
    return (Script *) luaL_checkudata(L, index, SCRIPT_LIB);
}

static int 
script_new (lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    Script *script = lua_newscript(L, filename);
    lua_pushobject (L, script, SCRIPT_LIB);
    return 1;
}

static int
script_print (lua_State* L)
{
    Script *script = lua_checkscript(L, 1);
    lua_pushfstring(L, "%s %p", SCRIPT_LIB, script);
    return 1;
}

static const luaL_Reg script_methods[] = {
    {"__tostring", script_print},
    { NULL, NULL }
};

int 
luaopen_GoodsackAPI_Actor_Script (lua_State *L)
{
    /* create metatable */
    luaL_newmetatable(L, SCRIPT_LIB);

    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");

    /* register methods */
    luaL_setfuncs(L, script_methods, 0);

    /* Push a function: Script(...) => new script */
    lua_pushcfunction(L, script_new);

    return 1;
}
