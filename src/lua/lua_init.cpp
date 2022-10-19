#include "lua_init.hpp"

#include <stdlib.h>
#include <string.h>

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

// Testing
#include <ecs/lua/eventstore.hpp>
#include <ecs/lua/reg_system.hpp>

#include <lua/debug.h>
#include <lua/print.h>

int _lua_MyStruct_index(lua_State *L) {
    struct MyStruct *t = (struct MyStruct *)luaL_checkudata(L, 1, "MyStruct");
    //dumpstack(L, "index");
    const char *k = luaL_checkstring(L, -1);
    //dumpstack(L, "index");
    if(!strcmp(k, "value")) {
        lua_pushnumber(L, t->value);
    }
    else if(!strcmp(k, "rand")) {
        lua_pushnumber(L, t->rand);
    }
    else if(!strcmp(k, "subset")) {
        //lua_rawgeti(L, -1, 1);

        //luaL_checktype(L, -1, LUA_TTABLE);
        //luaL_getmetatable(L, "MyStruct");
        //lua_getfield(L, -1, "subset");
        //luaL_getsubtable(L, -1, "subset");
        //luaL_getmetafield(L, 1, "health");
        //
        //iterate_and_print(L, -1);

        //dumpstack(L, "note: subset gettable");
    }
    else {
        lua_pushnil(L);
    }
    return 1;
}

int _lua_MyStruct_newindex(lua_State *L) {
    struct MyStruct *t = (struct MyStruct *)luaL_checkudata(L, 1, "MyStruct");
    //dumpstack(L, "MyStruct_newindex");
    const char *k = luaL_checkstring(L, -2);
    //dumpstack(L, "MyStruct_newindex");

    if(!strcmp(k, "value")) {
        t->value = luaL_checknumber(L, -1);
        return 0;
    }
    else if(!strcmp(k, "rand")) {
        t->rand = luaL_checknumber(L, -1);
        return 0;
    }
    else {
        return luaL_argerror(
            L, -2, lua_pushfstring(L, "invalid option '%s'", k));
    }
}

void func(lua_State *L) {
    // Since this is allocated inside of Lua, it will be garbage collected,
    // so we don't need to worry about freeing it
    //
    // MyStruct_new()

    // Create an "entity" which will be a container for 'MyStruct' and value id
    lua_newtable(L);
    lua_pushstring(L, "id");
    lua_pushnumber(L, 0);
    lua_settable(L, -3);
    //lua_pop(L, 2);

    // Test for ID
    lua_getfield(L, -1, "id");
    lua_pop(L, 1);

    lua_pushstring(L, "data");
    struct MyStruct *var = (struct MyStruct *)lua_newuserdata(L, sizeof *var);
    luaL_setmetatable(L, "MyStruct");
    //dumpstack(L, "func");
    lua_settable(L, -3);
    //lua_settable(L, -3);
    var->value = 5;

    //var->subset = (float *)lua_newuserdata(L, 4);
    //luaL_setmetatable(L, "MyStruct.subset");

    lua_getglobal(L, "Update");
    //dumpstack(L, "push");
    lua_pushvalue(L, -2); // push `var` to lua, somehow

    if(CheckLua(L, lua_pcall(L, 1, 0, 0))) {
        // now, var->value has changed 
        printf("\nMyStruct value is: %f", var->value);
    }
}

void LuaTest(const char *file) {
    struct Player player = {"Undefined", 69};

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_luaprintlib(L);


    if (CheckLua(L, luaL_dofile(L, file))) {
#if 1
        // One-time setup that needs to happen before you first call MyStruct_new
        luaL_newmetatable(L, "MyStruct");
        lua_pushcfunction(L, _lua_MyStruct_index);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, _lua_MyStruct_newindex);
        lua_setfield(L, -2, "__newindex");
        lua_pop(L, 1);
    }

#endif

    func(L);

    printf("\nPlayer name: %s", player.name);
    printf("\n");

    lua_close(L);
}
//----------------------------------------------------------------------

void LuaInit(const char *file) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_luaprintlib(L);

    ECSEventStore::Initialize(L);

    // add the global C function (register system) to lua
    lua_register(L, "_ECS_RegisterSystem", Lua_ECSRegisterSystem);

    if(CheckLua(L, luaL_dofile(L, file))) {

        // entry from lua [function main()]
        lua_getglobal(L, "main");
        if(lua_isfunction(L, -1)) {
            CheckLua(L, lua_pcall(L, 0, 0, 0));
        }
        dumpstack(L, "before");

    ECSEventStore::ECSEvent(ECS_INIT);
    ECSEventStore::ECSEvent(ECS_UPDATE);
    dumpstack(L, "end");

    }

    lua_close(L);
}
