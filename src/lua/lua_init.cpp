#include "lua_init.h"

#include <string.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

static int CheckLua(lua_State *L, int r) {
    if(r != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        printf("[Lua Error]: %s", err);
        return 0;
    }
    return 1;
}

static void dumpstack (lua_State *L, const char *message) {
  printf("\n\n-------------------------\n");
  printf("[Lua] Stack Dump Begin (%s)\n", message);
  int top = lua_gettop(L);
  int s = -1;
  for (int i=top; i >= 1; i--) {
    printf("%d\t%s\t", s, luaL_typename(L,i));
    switch (lua_type(L, i)) {
      case LUA_TNUMBER:
        printf("%g\n",lua_tonumber(L,i));
        break;
      case LUA_TSTRING:
        printf("%s\n",lua_tostring(L,i));
        break;
      case LUA_TBOOLEAN:
        printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
        break;
      case LUA_TNIL:
        printf("%s\n", "nil");
        break;
      default:
        printf("%p\n",lua_topointer(L,i));
        break;
    }
    s--;
  }
  printf("\n[Lua] Stack Dump End\n");
  printf("-------------------------\n\n");
}

int _lua_MyStruct_subset_index(lua_State *L) {
    luaL_checkudata(L, 1, "MyStruct.subset");
    const char *k = lua_tostring(L, -1);

    if(!strcmp(k, "health")) {
        //lua_pushnumber(L, t->value);
        dumpstack(L, "got health");
    }
    else {
        lua_pushnil(L);
    }

    return 1;
}

static void iterate_and_print(lua_State *L, int index)
{
    // Push another reference to the table on top of the stack (so we know
    // where it is, and this function can work for negative, positive and
    // pseudo indices
    lua_pushvalue(L, index);
    // stack now contains: -1 => table
    lua_pushnil(L);
    // stack now contains: -1 => nil; -2 => table
    while (lua_next(L, -2))
    {
        // stack now contains: -1 => value; -2 => key; -3 => table
        // copy the key so that lua_tostring does not modify the original
        lua_pushvalue(L, -2);
        // stack now contains: -1 => key; -2 => value; -3 => key; -4 => table
        const char *key = lua_tostring(L, -1);
        const char *value = lua_tostring(L, -2);
        printf("%s => %s\n", key, value);
        // pop value + copy of key, leaving original key
        lua_pop(L, 2);
        // stack now contains: -1 => key; -2 => table
    }
    // stack now contains: -1 => table (when lua_next returns 0 it pops the key
    // but does not push anything.)
    // Pop table
    lua_pop(L, 1);
    // Stack is now the same as it was on entry to this function
}

int _lua_MyStruct_index(lua_State *L) {
    struct MyStruct *t = (struct MyStruct *)luaL_checkudata(L, 1, "MyStruct");
    dumpstack(L, "index");
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

        dumpstack(L, "note: subset gettable");
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
    dumpstack(L, "push");
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

    if (CheckLua(L, luaL_dofile(L, file))) {
#if 1
        // One-time setup that needs to happen before you first call MyStruct_new
        luaL_newmetatable(L, "MyStruct");
        lua_pushcfunction(L, _lua_MyStruct_index);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, _lua_MyStruct_newindex);
        lua_setfield(L, -2, "__newindex");
        lua_pop(L, 1);

        luaL_newmetatable(L, "MyStruct.subset");
        lua_pushcfunction(L, _lua_MyStruct_subset_index);
        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);

        lua_getglobal(L, "system");
        //lua_pushstring(L, "test");
        //lua_gettable(L, -2);
        lua_getfield(L, -1, "test");
        dumpstack(L, "system getglobal");
        if(lua_isfunction(L, -1)) printf("Function");
        lua_pop(L, 2);

        //const char *k = luaL_checkstring(L, -2);
        //dumpstack(L, "system getglobal");
    }

#endif

    func(L);

    printf("\nPlayer name: %s", player.name);
    printf("\n");

    lua_close(L);
}

