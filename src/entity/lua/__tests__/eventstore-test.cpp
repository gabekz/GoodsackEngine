#include <gtest/gtest.h>

#include <stdlib.h>

#include <entity/lua/eventstore.hpp>
#include <entity/v1/ecs.h>

#include <util/lua_deps.h>

#include <entity/ecsdefs.h>

struct TestEventStore : testing::Test
{

    TestEventStore() {};

    virtual ~TestEventStore() {}
};

// TODO: Disabled due to file-pathing issues.
TEST_F(TestEventStore, DISABLED_Initialization_And_Event)
{
    lua_State *L = luaL_newstate();
    entity::LuaEventStore::Initialize(L, NULL);
}
