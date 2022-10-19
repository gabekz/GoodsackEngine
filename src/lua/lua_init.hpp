#ifndef H_LUA_INIT
#define H_LUA_INIT

#include <util/sysdefs.h>

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

    int *functions2;
    int n_functions2;
};


void LuaTest(const char* file);
void LuaInit(const char *file);

#endif // H_LUA_INIT
