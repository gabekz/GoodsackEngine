#include "lua_init.h"

#include <stdlib.h>
#include <string.h>


extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
    #include <core/ecs/ecs_lua.h>
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

static struct Lua_ECSEventStore* create_ecs_eventstore(lua_State *L) {
    struct Lua_ECSEventStore *store = 
        (struct Lua_ECSEventStore *)malloc(sizeof(struct Lua_ECSEventStore));

    // create function store
    lua_newtable(L);
    dumpstack(L, "new");

    lua_pushstring(L, "start");
    lua_newtable(L);
    lua_settable(L, -3);
    dumpstack(L, "settable start");

    lua_pushstring(L, "update");
    lua_newtable(L);
    lua_settable(L, -3);
    dumpstack(L, "settable update");

    store->tableId = luaL_ref(L, LUA_REGISTRYINDEX);
    dumpstack(L, "settable tableId");

    // going to realloc anyway
    store->functions = (int *)malloc(sizeof(int));
    store->functions2 = (int *)malloc(sizeof(int));
    store->n_functions = 0;
    store->n_functions2 = 0;

    return store;
}

static void add_ecs_eventstore(const char *event, int fn) {
    if(!strcmp(event, "start")) {
        store->n_functions++;
        store->functions = (int *)realloc(store->functions, store->n_functions * sizeof(int));
        store->functions[store->n_functions-1] = fn; // send function to list
    }
    else if(!strcmp(event, "update")) {
        store->n_functions2++;
        store->functions2 = (int *)realloc(store->functions2, store->n_functions2 * sizeof(int));
        store->functions2[store->n_functions2-1] = fn; // send function to list

    }
}

int _lua_test_ecs_register_system(lua_State *L) {
    lua_rawgeti(L,LUA_REGISTRYINDEX, store->tableId); // retrieve table for functions
    // <args>, register-table

    /* start() function */

    lua_getfield(L, -1, "start");
    // <args>, register-table, table
    lua_getfield(L, -3, "start"); // get "start" function from <args>
    // <args>, register-table, table, function

    if(lua_isfunction(L, -1)) {
        int f = luaL_ref(L, -2); // register to table "start"

        add_ecs_eventstore("start", f);
    }
    else {
        // we want to pop this if not a function. Registering as
        // reference will pop this already.
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    // <args>, register-table

    /* update() function */

    lua_getfield(L, -1, "update");
    // <args>, register-table, table
    lua_getfield(L, -3, "update"); // get "update" function from <args>
    // <args>, register-table, table, function

    if(lua_isfunction(L, -1)) {
        int f = luaL_ref(L, -2); // register to table "start"

        add_ecs_eventstore("update", f);
    }
    else {
        // we want to pop this if not a function. Registering as
        // reference will pop this already.
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    // <args>, register-table

    lua_pop(L, 3);
    // <empty>
    return 1;
}

void LuaInit(const char *file) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_luaprintlib(L);

    // TODO: change from gloal struct to something.. beter.
    store = create_ecs_eventstore(L);

    struct TLua_ECSEventStore *t = Tcreate_ecs_eventstore(L);
    ecs_lua_event(L, t, ECS_INIT, (Entity){.id = 0});

    // add the global C function (register system) to lua
    lua_register(L, "_ECS_RegisterSystem", _lua_test_ecs_register_system);

    if(CheckLua(L, luaL_dofile(L, file))) {

        // entry from lua [function main()]
        lua_getglobal(L, "main");
        if(lua_isfunction(L, -1)) {
            CheckLua(L, lua_pcall(L, 0, 0, 0));
        }
        dumpstack(L, "before");

        //dumpstack(L, "before");
        //printf("Calling stored functions from C:\n");
        //lua_rawgeti(L, LUA_REGISTRYINDEX, store->tableId); // retrieve function table

        lua_rawgeti(L,LUA_REGISTRYINDEX, store->tableId); // retrieve function table

        // start
        lua_getfield(L, -1, "start"); // retrieve all "start" functions
        for(int i = 0; i < store->n_functions; i++) {
            //dumpstack(L, "START collect function");
            lua_rawgeti(L, -1, store->functions[i]); // retreive function
            //dumpstack(L, "MIDDLE collect function");
            if(lua_isfunction(L, -1)) {
                lua_pushnumber(L, 9);
                (CheckLua(L, lua_pcall(L, 1, 0, 0)));
            }
        }
        lua_pop(L, 2);
        // <empty>

        lua_rawgeti(L,LUA_REGISTRYINDEX, store->tableId); // retrieve function table
        // update 
        lua_getfield(L, -1, "update"); // retrieve all "start" functions
        for(int i = 0; i < store->n_functions2; i++) {
            //dumpstack(L, "START collect function");
            lua_rawgeti(L, -1, store->functions2[i]); // retreive function
            //dumpstack(L, "MIDDLE collect function");
            if(lua_isfunction(L, -1)) {
                lua_pushnumber(L, 12);
                (CheckLua(L, lua_pcall(L, 1, 0, 0)));
            }
        }
        lua_pop(L, 2);
        // <empty>

        dumpstack(L, "end");
    }

// cleanup
    free(store->functions);
    free(store->functions2);
    free(store);

    lua_close(L);
}
