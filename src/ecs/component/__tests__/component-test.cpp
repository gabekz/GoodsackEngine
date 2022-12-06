/*T*********************************************************************
 * FILENAME:        component-test.cpp
 *
 * UNIT:            ECS Component Loader
 * UNIT TYPE:       Functionality
 *
 * SYNOPSIS:
 *      Testing the ECS Component Loading functionality, in that it
 *      is able to store and give user-created data.
 *
 *T*/


#include <stdlib.h>
#include <gtest/gtest.h>

#include <util/lua_deps.h>
#include <ecs/component/component.hpp>

#include <ecs/component/loader.hpp>

#include <util/maths.h>

struct ComponentLoaderTest : testing::Test {
    const char *rawComponentData = R"(
{
  "ComponentTransform": {
    "vec3": [ "position", "axisUp" ],
    "int": [ "uboId" ]
  },
 
  "ComponentCamera": {
    "vec3": [ "position", "rotation", "scale" ],
    "float": [ "fov", "speed", "sensitivity" ],
    "mat4": [ "view", "projection" ]
  }
}
)";

    std::map<std::string, ecs::ComponentLayout *> m_Layouts;

    ComponentLoaderTest() {
        m_Layouts = ecs::ParseComponents(rawComponentData, 1);
    }
};

TEST_F(ComponentLoaderTest, Reads_Writes_Numeric) {
    ecs::Component *p = new ecs::Component(*m_Layouts["ComponentTransform"]);
    ecs::Component *p2 = new ecs::Component(*m_Layouts["ComponentCamera"]);

    int value, newValue = 70;
    ASSERT_TRUE(p->SetVariable("uboId", &newValue));
    ASSERT_TRUE(p->GetVariable("uboId", &value));
    EXPECT_EQ(value, newValue);

    float fvalue, fnewValue = 12.13f;
    ASSERT_TRUE(p2->SetVariable("speed", &fnewValue));
    ASSERT_TRUE(p2->GetVariable("speed", &fvalue));
    EXPECT_EQ(fvalue, fnewValue);

    delete(p);
    delete(p2);
}

TEST_F(ComponentLoaderTest, Reads_Writes_Vec3) {
    ecs::Component *p = new ecs::Component(*m_Layouts["ComponentCamera"]);

    vec3 vectorA = {0.25f, 5.8f, 1.0f};
    vec3 vectorB = GLM_VEC3_ZERO_INIT;

    ASSERT_TRUE(p->SetVariable("position", vectorA));
    ASSERT_TRUE(p->GetVariable("position", &vectorB));

    EXPECT_EQ(vectorA[0], vectorB[0]);
    EXPECT_EQ(vectorA[1], vectorB[1]);
    EXPECT_EQ(vectorA[2], vectorB[2]);
    delete(p);
}

TEST_F(ComponentLoaderTest, Reads_Writes_Matrix_Data) {
    ecs::Component *p = new ecs::Component(*m_Layouts["ComponentCamera"]);

    mat4 matrixA = GLM_MAT4_IDENTITY_INIT;
    mat4 matrixB = GLM_MAT4_ZERO_INIT;
    p->SetVariable("view", &matrixA);
    ASSERT_TRUE(p->GetVariable("view", &matrixB));

    //ASSERT_THAT(*matrixB, testing::Each(testing::AllOf(testing::Gt(-1), testing::Lt(20))));

    EXPECT_EQ(matrixA[0][0], matrixB[0][0]);
    EXPECT_EQ(matrixA[0][1], matrixB[0][1]);
    EXPECT_EQ(matrixA[0][2], matrixB[0][2]);
    EXPECT_EQ(matrixA[0][3], matrixB[0][3]);

    EXPECT_EQ(matrixA[1][0], matrixB[1][0]);
    EXPECT_EQ(matrixA[1][1], matrixB[1][1]);
    EXPECT_EQ(matrixA[1][2], matrixB[1][2]);
    EXPECT_EQ(matrixA[1][3], matrixB[1][3]);

    EXPECT_EQ(matrixA[2][0], matrixB[2][0]);
    EXPECT_EQ(matrixA[2][1], matrixB[2][1]);
    EXPECT_EQ(matrixA[2][2], matrixB[2][2]);
    EXPECT_EQ(matrixA[2][3], matrixB[2][3]);

    EXPECT_EQ(matrixA[3][0], matrixB[3][0]);
    EXPECT_EQ(matrixA[3][1], matrixB[3][1]);
    EXPECT_EQ(matrixA[3][2], matrixB[3][2]);
    EXPECT_EQ(matrixA[3][3], matrixB[3][3]);

    delete(p);
}

/*
TEST_F(ComponentLoaderTest, Reads_Writes_Structs) {
    EXPECT_EQ(1, 2);
}
*/

/*
TEST_F(ComponentLoaderTest, Create_Undefined_Component) {
    ecs::Component *p = new ecs::Component(*m_Layouts["ComponentUndefined"]);
}
*/