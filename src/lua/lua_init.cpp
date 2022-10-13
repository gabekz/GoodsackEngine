#include "lua_init.h"

#include <stdlib.h>
#include <string.h>

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

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

static struct Lua_ECSEventStore *store;

int _lua_test_ecs_register_system(lua_State *L) {
    lua_rawgeti(L,LUA_REGISTRYINDEX, store->tableId); // retrieve table for functions

    lua_getfield(L, -2, "start");
    if(lua_isfunction(L, -1)) {
        int f = luaL_ref(L,-2);
        lua_pop(L, 1);

        store->n_functions++;
        store->functions = (int *)realloc(store->functions, store->n_functions * sizeof(int));
        store->functions[store->n_functions-1] = f; // send function to list
    }

    lua_pop(L, 1);
    return 1;
}

static struct Lua_ECSEventStore* create_ecs_eventstore(lua_State *L) {

    struct Lua_ECSEventStore *store = 
        (struct Lua_ECSEventStore *)malloc(sizeof(struct Lua_ECSEventStore));

    // create function store
    lua_newtable(L);
    store->tableId = luaL_ref(L, LUA_REGISTRYINDEX);

    // going to realloc anyway
    store->functions = (int *)malloc(sizeof(int));
    store->n_functions = 0;

    return store;
}

void LuaInit(const char *file) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_luaprintlib(L);

    // TODO: change from gloal struct to something.. beter.
    store = create_ecs_eventstore(L);

    // add the global C function (register system) to lua
    lua_register(L, "_ECS_RegisterSystem", _lua_test_ecs_register_system);

    if(CheckLua(L, luaL_dofile(L, file))) {

        // entry from lua [function main()]
        lua_getglobal(L, "main");
        if(lua_isfunction(L, -1)) {
            CheckLua(L, lua_pcall(L, 0, 1, 0));
        }
        dumpstack(L, "before");

        //dumpstack(L, "before");
        //printf("Calling stored functions from C:\n");
        //lua_rawgeti(L, LUA_REGISTRYINDEX, store->tableId); // retrieve function table

        lua_rawgeti(L,LUA_REGISTRYINDEX, store->tableId); // retrieve function table
        for(int i = 0; i < store->n_functions; i++) {
            //dumpstack(L, "START collect function");
            printf("\nout out %d", store->functions[i]);
            lua_rawgeti(L, -1, store->functions[i]); // retreive function
            //dumpstack(L, "MIDDLE collect function");
            if(lua_isfunction(L, -1)) {
                lua_pushnumber(L, 9);
                (CheckLua(L, lua_pcall(L, 1, 0, 0)));
            }
        }
        lua_pop(L, 1); // pop function-table
        dumpstack(L, "end");
    }

// cleanup
    free(store->functions);
    free(store);

    lua_close(L);
}
