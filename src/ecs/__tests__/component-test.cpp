/*T*********************************************************************
 * FILENAME:        component-test.cpp
 *
 * UNIT:            ECS Component
 * UNIT TYPE:       Object
 *
 * SYNOPSIS:
 *      Testing the ECS Component functionality, in that it
 *      is able to store and give user-created data.
 *
 *T*/


#include <stdlib.h>
#include <gtest/gtest.h>

#include <util/lua_deps.h>
#include <ecs/component/component.hpp>

struct ComponentTest : testing::Test {

    ecs::ComponentLayout *m_Layout;

    ComponentTest() {
        m_Layout = new ecs::ComponentLayout("Mesh");
    }
};

// TEST: MyFirst Test
// DESC: Tests the functionality
//
TEST_F(ComponentTest, MyFirstTest) {
    ecs::Component *c = new ecs::Component(*m_Layout);
    EXPECT_EQ(c->getName(), "Mesh");
    EXPECT_EQ(c->getName(), "MesC");
}

// TEST: MyFirst Test
// DESC: Tests the functionality
//
TEST_F(ComponentTest, SecondTest) {
    ecs::ComponentLayout *mesh = new ecs::ComponentLayout("Mesh");
    ecs::Component *c = new ecs::Component(*mesh);
    EXPECT_EQ(c->getName(), "Mesh");
}
