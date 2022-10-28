#include <gtest/gtest.h>

#include <stdlib.h>

#include <ecs/ecs.h>
#include <ecs/component/component.hpp>

#include <util/lua_deps.h>

struct ComponentTest : testing::Test {
};

TEST_F(ComponentTest, MyFirstTest) {
    ecs::ComponentLayout *mesh = new ecs::ComponentLayout("Mesh");
    ecs::Component *c = new ecs::Component(*mesh);
    EXPECT_EQ(c->getName(), "Mesh");
}

TEST_F(ComponentTest, SecondTest) {
    ecs::ComponentLayout *mesh = new ecs::ComponentLayout("Mesh");
    ecs::Component *c = new ecs::Component(*mesh);
    EXPECT_EQ(c->getName(), "Mesh");
}
