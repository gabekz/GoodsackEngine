/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

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

#include "util/lua_deps.h"
#include "util/maths.h"

#include "entity/component/ecs_component.hpp"
#include "entity/component/ecs_component_layout_loader.hpp"

#include <gtest/gtest.h>

using namespace entity;

struct ComponentLoaderTest : testing::Test
{
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
  },

  "ComponentCameraExpanded": {
    "float": [ "fov", "speed", "sensitivity" ],
    "vec3": [ "scale"],
    "mat4": [ "view", "projection" ],
    "vec3": [ "zVector" ]
  },

  "ComponentMat3Test": {
    "mat3": ["val"]
  },

  "ComponentSingle": {
    "int": [ "view"]
  },

  "ComponentWithResource": {
    "int": [ "myInt1"],
    "Resource": ["myResource"]
  }
}
)";

    std::map<std::string, ECSComponentLayout *> m_Layouts;

    ComponentLoaderTest()
    {
        m_Layouts =
          entity::component::parse_components_from_json(rawComponentData, 1);
    }
    virtual ~ComponentLoaderTest() { m_Layouts.clear(); }
};

TEST_F(ComponentLoaderTest, Reads_Writes_Numeric)
{
    ECSComponent *p  = new ECSComponent(*m_Layouts["ComponentTransform"]);
    ECSComponent *p2 = new ECSComponent(*m_Layouts["ComponentCamera"]);

    int value, newValue = 70;
    ASSERT_TRUE(p->SetVariable("uboId", &newValue));
    ASSERT_TRUE(p->GetVariable("uboId", &value));
    EXPECT_EQ(value, newValue);

    float fvalue, fnewValue = 12.13f;
    ASSERT_TRUE(p2->SetVariable("speed", &fnewValue));
    ASSERT_TRUE(p2->GetVariable("speed", &fvalue));
    EXPECT_EQ(fvalue, fnewValue);

    // delete (p);
    // delete (p2);
}

TEST_F(ComponentLoaderTest, Reads_Writes_Vec3)
{
    ECSComponent *p = new ECSComponent(*m_Layouts["ComponentCamera"]);

    vec3 vectorA = {0.25f, 5.8f, 1.0f};
    vec3 vectorB = GLM_VEC3_ZERO_INIT;

    ASSERT_TRUE(p->SetVariable("position", vectorA));
    ASSERT_TRUE(p->GetVariable("position", &vectorB));

    EXPECT_EQ(vectorA[0], vectorB[0]);
    EXPECT_EQ(vectorA[1], vectorB[1]);
    EXPECT_EQ(vectorA[2], vectorB[2]);
    // delete (p);
}

TEST_F(ComponentLoaderTest, Reads_Writes_Vec3_Exp)
{
    ECSComponent *p = new ECSComponent(*m_Layouts["ComponentCameraExpanded"]);

    vec3 vectorA = {0.25f, 5.8f, 1.0f};
    vec3 vectorB = GLM_VEC3_ZERO_INIT;

    ASSERT_TRUE(p->SetVariable("zVector", vectorA));
    ASSERT_TRUE(p->GetVariable("zVector", &vectorB));

    EXPECT_EQ(vectorA[0], vectorB[0]);
    EXPECT_EQ(vectorA[1], vectorB[1]);
    EXPECT_EQ(vectorA[2], vectorB[2]);
    // delete (p);
}

TEST_F(ComponentLoaderTest, Reads_Writes_Matrix_3x3_Data)
{
    ECSComponent *p = new ECSComponent(*m_Layouts["ComponentMat3Test"]);

    mat3 matrixA = GLM_MAT3_IDENTITY_INIT;
    mat3 matrixB = GLM_MAT3_ZERO_INIT;
    p->SetVariable("val", &matrixA);
    ASSERT_TRUE(p->GetVariable("val", &matrixB));

    EXPECT_EQ(matrixA[0][0], matrixB[0][0]);
    EXPECT_EQ(matrixA[0][1], matrixB[0][1]);
    EXPECT_EQ(matrixA[0][2], matrixB[0][2]);

    EXPECT_EQ(matrixA[1][0], matrixB[1][0]);
    EXPECT_EQ(matrixA[1][1], matrixB[1][1]);
    EXPECT_EQ(matrixA[1][2], matrixB[1][2]);

    EXPECT_EQ(matrixA[2][0], matrixB[2][0]);
    EXPECT_EQ(matrixA[2][1], matrixB[2][1]);
    EXPECT_EQ(matrixA[2][2], matrixB[2][2]);
}

TEST_F(ComponentLoaderTest, Reads_Writes_Matrix_4x4_Data)
{
    ECSComponent *q = new ECSComponent(*m_Layouts["ComponentCamera"]);

    mat4 matrixA = GLM_MAT4_IDENTITY_INIT;
    mat4 matrixB = GLM_MAT4_ZERO_INIT;
    q->SetVariable("view", &matrixA);
    ASSERT_TRUE(q->GetVariable("view", &matrixB));

    // ASSERT_THAT(*matrixB, testing::Each(testing::AllOf(testing::Gt(-1),
    // testing::Lt(20))));

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

    // delete (p);
}

TEST_F(ComponentLoaderTest, Reads_Writes_Mixed)
{
    ECSComponent *cmp = new ECSComponent(*m_Layouts["ComponentCameraExpanded"]);

    mat4 matrixA = GLM_MAT4_IDENTITY_INIT;
    mat4 matrixB = GLM_MAT4_ZERO_INIT;

    cmp->SetVariable("view", &matrixA);
    ASSERT_TRUE(cmp->GetVariable("view", &matrixB));
    EXPECT_EQ(matrixA[1][0], matrixB[1][0]);
    EXPECT_EQ(matrixA[1][1], matrixB[1][1]);

    vec3 vectorA = {0.2f, 0.2f, 0.2f};
    vec3 vectorB = GLM_VEC3_ZERO_INIT;
    ASSERT_TRUE(cmp->SetVariable("zVector", vectorA));
    ASSERT_TRUE(cmp->GetVariable("zVector", &vectorB));
    EXPECT_EQ(vectorA[0], vectorB[0]);
    EXPECT_EQ(vectorA[1], vectorB[1]);
    EXPECT_EQ(vectorA[2], vectorB[2]);

#if 0
    vec3 vectorC = {0.25f, 5.8f, 1.0f};
    vec3 vectorD = GLM_VEC3_ZERO_INIT;
    ASSERT_TRUE(cmp->SetVariable("position", vectorC));
    ASSERT_TRUE(cmp->GetVariable("position", &vectorD));
#endif
}

TEST_F(ComponentLoaderTest, Maps_To_C_Struct)
{
    struct C_ComponentSingle
    {
        int view;
    };

    C_ComponentSingle *cStruct =
      (C_ComponentSingle *)malloc(sizeof(C_ComponentSingle));
    cStruct->view = 32;

    ECSComponent *p = new ECSComponent(cStruct, *m_Layouts["ComponentSingle"]);
// p->MapFromExisting(cStruct, *m_Layouts["ComponentSingle"]);

// Check size of C Struct against generated component
#if 0 // Needs alignment
    ASSERT_EQ((size_t)sizeof(C_ComponentSingle),
              m_Layouts["ComponentSingle"]->getSizeReq());
#endif

    // Check initial Get
    int fetched_view = 0;
    ASSERT_TRUE(p->GetVariable("view", &fetched_view));
    ASSERT_EQ(fetched_view, 32);

    // Check Set
    int new_view = 64;
    p->SetVariable("view", &new_view);
    ASSERT_TRUE(p->GetVariable("view", &fetched_view));
    ASSERT_EQ(fetched_view, 64);
}

TEST_F(ComponentLoaderTest, Reads_Writes_voidResource)
{
    struct FooResource
    {
        int value;
        float value2;
    };

    ECSComponent *q = new ECSComponent(*m_Layouts["ComponentWithResource"]);

    FooResource *foo = (FooResource *)malloc(sizeof(FooResource));
    foo->value       = 100;
    foo->value2      = 5.15f;

    ASSERT_TRUE(q->SetVariable("myResource", &foo));

    FooResource *fetched_foo = nullptr;

    ASSERT_TRUE(q->GetVariable("myResource", &fetched_foo));
    ASSERT_EQ(fetched_foo, foo);
    ASSERT_EQ(fetched_foo->value, 100);
    ASSERT_EQ(fetched_foo->value2, 5.15f);
}
