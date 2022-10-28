#include <gtest/gtest.h>

#include <stdlib.h>

#include <ecs/ecs.h>
#include <ecs/lua/eventstore.hpp>

#include <util/lua_deps.h>

struct TestEventStore : testing::Test {

    TestEventStore() {

    };

    virtual ~TestEventStore() {

    }

};

TEST_F(TestEventStore, Initialization) {
    EXPECT_EQ(1, 2);
}
