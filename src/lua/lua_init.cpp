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

int MyStruct_index(lua_State *L) {
    struct MyStruct *t = (struct MyStruct *)luaL_checkudata(L, 1, "MyStruct");
    const char *k = luaL_checkstring(L, 2);
    dumpstack(L, "MyStruct_index");
    if(!strcmp(k, "value")) {
        lua_pushnumber(L, t->value);
    }
    else if(!strcmp(k, "rand")) {
        lua_pushnumber(L, t->rand);
    }
    else {
        lua_pushnil(L);
    }
    return 1;
}

int MyStruct_newindex(lua_State *L) {
    struct MyStruct *t = (struct MyStruct *)luaL_checkudata(L, 1, "MyStruct");
    const char *k = luaL_checkstring(L, 2);

    dumpstack(L, "MyStruct_newindex");

    if(!strcmp(k, "value")) {
        t->value = luaL_checknumber(L, 3);
        return 0;
    }
    else if(!strcmp(k, "rand")) {
        t->rand = luaL_checknumber(L, 3);
        return 0;
    }
    else {
        return luaL_argerror(
            L, 2, lua_pushfstring(L, "invalid option '%s'", k));
    }
}

void func(lua_State *L) {
    // Since this is allocated inside of Lua, it will be garbage collected,
    // so we don't need to worry about freeing it
    //
    // MyStruct_new()
    struct MyStruct *var = (struct MyStruct *)lua_newuserdata(L, sizeof *var);
    luaL_setmetatable(L, "MyStruct");
    var->value = 5;

    lua_getglobal(L, "Update");
    lua_pushvalue(L, -2); // push `var` to lua, somehow

    lua_call(L, 1, 0); // prints "5"

    // now, var->value has changed 
    printf("\nMyStruct value is: %f", var->value);
}

void LuaTest(const char *file) {
    struct Player player = {"Undefined", 69};

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (CheckLua(L, luaL_dofile(L, file))) {
#if 0
        lua_getglobal(L, "a");
        if(lua_isnumber(L, -1)) {
            printf("%f", (float)lua_tonumber(L, -1));
        }

        lua_getglobal(L, "PlayerName");
        if(lua_isstring(L, -1)) {
            player.name = lua_tostring(L, -1);
        }

        lua_getglobal(L, "Player");
        if(lua_istable(L, -1)) {
            lua_pushstring(L, "Name");
            lua_gettable(L, -2);
            player.name = lua_tostring(L, -1);
            lua_pop(L, 1);
        }

        lua_getglobal(L, "Add");
        if(lua_isfunction(L, -1)) {
            lua_pushnumber(L, 52);
            lua_pushnumber(L, 12);

            if(CheckLua(L, lua_pcall(L, 2, 1, 0))) {
                printf("\nFrom C, Lua Add(): %f.", lua_tonumber(L, -1));
            }
        }

        lua_getglobal(L, "Update");
        if(lua_isfunction(L, -1)) {
            lua_pushlightuserdata(L, &player);

            if(CheckLua(L, lua_pcall(L, 1, 0, 0))) {
                printf("\nFrom C, Lua Update(&player): %s.", player.name);
            }
        }
#endif

#if 1
        // One-time setup that needs to happen before you first call MyStruct_new
        luaL_newmetatable(L, "MyStruct");
        lua_pushcfunction(L, MyStruct_index);
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, MyStruct_newindex);
        lua_setfield(L, -2, "__newindex");
        //dumpstack(L);
        lua_pop(L, 1);
        //dumpstack(L);
    }

#endif

    func(L);

    printf("\nPlayer name: %s", player.name);
    printf("\n");

    lua_close(L);
}

