#ifndef H_LUA_INIT
#define H_LUA_INIT

#include <util/sysdefs.h>

extern "C" {
    #include "lua.h" 
}

struct Player {
    const char* name;
    int age;
};

struct MyStruct {
    float value;
    int rand;

    struct {
        int health;

    } subset;
};

struct Lua_ECSEventStore {
    int tableId;

    int *functions;
    int n_functions;
};


void LuaTest(const char* file);
void LuaInit(const char *file);

int CheckLua(lua_State *L, int r);
void dumpstack (lua_State *L, const char *message);

extern int luaopen_luaprintlib(lua_State *L);


#endif // H_LUA_INIT
