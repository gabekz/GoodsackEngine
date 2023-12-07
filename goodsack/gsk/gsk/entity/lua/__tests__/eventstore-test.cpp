/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "util/lua_deps.h"

#include "entity/ecs.h"
#include "entity/ecsdefs.h"
#include "entity/lua/eventstore.hpp"


// TODO: Move to thirdparty directive - gabekz/GoodsackEngine#19
#include <gtest/gtest.h>

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
