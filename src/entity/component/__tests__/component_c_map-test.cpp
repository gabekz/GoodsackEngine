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

#include <gtest/gtest.h>
#include <stdlib.h>

#include <entity/component/ecs_component.hpp>
#include <util/lua_deps.h>

#include <entity/component/ecs_component_layout_loader.hpp>

#include <util/maths.h>

using namespace entity;

struct ComponentCMapTest : testing::Test
{

    const char *rawComponentData = R"(
{
  "Transform": {
    "vec3": [ "position", "orientation", "scale" ],
    "mat4": [ "model" ],
    "bool": [ "hasParent" ],
    "Resource": [ "parent" ]
  }
}
)";

    std::map<std::string, ECSComponentLayout *> m_Layouts;

struct CmpTransform
{
    ui32 hasParent;
    mat4 model;
    vec3 orientation;
    void *parent;
    vec3 position;
    vec3 scale;
};


    ComponentCMapTest()
    {
        m_Layouts =
          entity::component::parse_components_from_json(rawComponentData, 1);
    }
    virtual ~ComponentCMapTest() { m_Layouts.clear(); }
};

TEST_F(ComponentCMapTest, Reads_Writes_Stuff)
{
    /*
    CmpTransform *transform =
      (CmpTransform *)malloc(sizeof(CmpTransform));
      */
    CmpTransform transform = (CmpTransform{.position = {32, 32, 32}});

    ECSComponent *cmp =
      new ECSComponent(&transform, *m_Layouts["ComponentTransform"]);

    vec3 vectorB = GLM_VEC3_ZERO_INIT;
    ASSERT_TRUE(cmp->GetVariable("position", &vectorB));
    EXPECT_EQ(transform.position[0], vectorB[0]);
    EXPECT_EQ(transform.position[1], vectorB[1]);
    EXPECT_EQ(transform.position[2], vectorB[2]);

    // delete (p);
    // delete (p2);
}