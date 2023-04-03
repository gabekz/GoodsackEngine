#ifndef HPP_REG_SYSTEM
#define HPP_REG_SYSTEM

#include <util/lua_deps.h>

namespace entity {

extern "C" {
int
Lua_ECSRegisterSystem(lua_State *L);
}
} // namespace entity

#endif // HPP_REG_SYSTEM
