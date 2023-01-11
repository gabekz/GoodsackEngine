#include <gtest/gtest.h>

#include <stdlib.h>

#include <ecs/ecs.h>
#include <ecs/lua/eventstore.hpp>

#include <util/lua_deps.h>

#include <ecs/ecsdefs.h>

struct TestEventStore : testing::Test
{

    TestEventStore() {};

    virtual ~TestEventStore() {}
};

// TODO: Disabled due to file-pathing issues.
TEST_F(TestEventStore, DISABLED_Initialization_And_Event)
{
    lua_State *L = luaL_newstate();
    ecs::LuaEventStore::Initialize(L);
}
