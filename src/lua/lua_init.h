#ifndef H_LUA_INIT
#define H_LUA_INIT

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

void LuaTest(const char* file);

#endif // H_LUA_INIT
