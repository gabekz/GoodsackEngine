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

TEST_F(TestEventStore, Initialization_And_Event)
{
    lua_State *L = luaL_newstate();
    ecs::LuaEventStore::Initialize(L);
}
