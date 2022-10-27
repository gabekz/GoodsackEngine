#include <gtest/gtest.h>
#include <ecs/ecs.h>

#include <stdlib.h>

TEST(HELLOGROUP, TestName) {
    Entity e = { .id = 32 };
    EXPECT_EQ(e.id, 32);
}
