#ifndef H_LUA_INIT
#define H_LUA_INIT

#include <entity/v1/ecs.h>
#include <util/sysdefs.h>

#define LUA_INIT_FILE_PATH "gsk://../gsk/gsk/api/lua/init.lua"

void
LuaInit(const char *file, ECS *ecs);

#endif // H_LUA_INIT
