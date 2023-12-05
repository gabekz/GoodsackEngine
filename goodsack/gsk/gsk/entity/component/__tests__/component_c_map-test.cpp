/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

/*T*********************************************************************
 * FILENAME:        component_c_map-test.cpp
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
#include "util/sysdefs.h"

#include "entity/component/ecs_component.hpp"
#include "entity/component/ecs_component_layout_loader.hpp"

#include <gtest/gtest.h>

#ifndef CACHE_LINE
#define CACHE_LINE ECS_COMPONENTS_ALIGN_BYTES
#endif // CACHE_LINE

using namespace entity;

struct ComponentCMapTest : testing::Test
{

    const char *rawComponentData = R"(
{
  "ComponentTransform": {
    "vec3": [ "position", "orientation", "scale" ],
    "mat4": [ "model" ],
    "bool": [ "hasParent" ],
    "Resource": [ "parent" ]
  }
}
)";

    std::map<std::string, ECSComponentLayout *> m_Layouts;

    typedef struct CmpTransform
    {
        CACHE_ALIGN(u16 hasParent);
        CACHE_ALIGN(mat4 model);
        CACHE_ALIGN(vec3 orientation);
        CACHE_ALIGN(void *parent);
        CACHE_ALIGN(vec3 position);
        CACHE_ALIGN(vec3 scale);
    } CmpTransform;

    ComponentCMapTest()
    {
        m_Layouts =
          entity::component::parse_components_from_json(rawComponentData, 1);
    }
    virtual ~ComponentCMapTest() { m_Layouts.clear(); }
};

TEST_F(ComponentCMapTest, Reads_Writes_Stuff)
{
    CmpTransform *transform = (CmpTransform *)malloc(sizeof(CmpTransform));

    ECSComponent *cmp =
      new ECSComponent(transform, *m_Layouts["ComponentTransform"]);

    // Check hasParent
    transform->hasParent = 0;
    u16 recHasParent     = -1;
    cmp->GetVariable("hasParent", &recHasParent);
    EXPECT_EQ(transform->hasParent, recHasParent);
    transform->hasParent = 1;
    cmp->GetVariable("hasParent", &recHasParent);
    EXPECT_EQ(transform->hasParent, recHasParent);

    // Check orientation
    vec3 newRot = {9, 30, -15};
    glm_vec3_copy(newRot, transform->orientation);
    vec3 vecRot = GLM_VEC3_ZERO_INIT;
    ASSERT_TRUE(cmp->GetVariable("orientation", &vecRot));
    EXPECT_EQ(transform->orientation[0], vecRot[0]);
    EXPECT_EQ(transform->orientation[1], vecRot[1]);
    EXPECT_EQ(transform->orientation[2], vecRot[2]);

    // Check position
    vec3 newPos = {12, 11, 10};
    glm_vec3_copy(newPos, transform->position);
    vec3 vecPos = GLM_VEC3_ZERO_INIT;
    ASSERT_TRUE(cmp->GetVariable("position", &vecPos));
    EXPECT_EQ(transform->position[0], vecPos[0]);
    EXPECT_EQ(transform->position[1], vecPos[1]);
    EXPECT_EQ(transform->position[2], vecPos[2]);

    // Check scale
    vec3 newScl = {-2, 2, 54.2f};
    glm_vec3_copy(newScl, transform->scale);
    vec3 vecScl = GLM_VEC3_ZERO_INIT;
    ASSERT_TRUE(cmp->GetVariable("scale", &vecScl));
    EXPECT_EQ(transform->scale[0], vecScl[0]);
    EXPECT_EQ(transform->scale[1], vecScl[1]);
    EXPECT_EQ(transform->scale[2], vecScl[2]);

    // delete (p);
    // delete (p2);
}
