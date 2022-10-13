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
    ui32 storeId;
};


void LuaTest(const char* file);
void LuaInit(const char *file);

int CheckLua(lua_State *L, int r);
void dumpstack (lua_State *L, const char *message);

extern int luaopen_luamylib(lua_State *L);


#endif // H_LUA_INIT
