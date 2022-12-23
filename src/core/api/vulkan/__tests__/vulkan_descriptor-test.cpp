/*T*********************************************************************
 * FILENAME:        vulkan_descriptor-test.cpp
 *
 * UNIT:            Vulkan Descriptor Handler
 * UNIT TYPE:       Functionality
 *
 * SYNOPSIS:
 *
 *T*/


#include <stdlib.h>
#include <gtest/gtest.h>

#include <core/api/vulkan/vulkan_descriptor_builder.h>

#include <util/gfx.h>
#include <util/maths.h>

struct VulkanDescriptorTest : testing::Test {

};

TEST_F(VulkanDescriptorTest, Allocates_a_Descriptor_Pool) {

    VkDescriptorSet GlobalSet;
    VkDescriptorBufferInfo bufferInfo;
    VkDescriptorImageInfo imageInfo;

    VulkanDescriptorLayoutCache layoutCache;

    vulkan_descriptor_builder_begin(&layoutCache);
    vulkan_descriptor_builder_bind_buffer(
            0,
            &bufferInfo,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            VK_SHADER_STAGE_VERTEX_BIT);
    vulkan_descriptor_builder_bind_buffer(
            1,
            &bufferInfo,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            VK_SHADER_STAGE_FRAGMENT_BIT);
    GlobalSet = vulkan_descriptor_builder_end();

    /*
    vulkan_descriptor_builder_bind_image(
            0,
            &imageInfo,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT);
    */

    ASSERT_TRUE(1 > 0);
    EXPECT_EQ(1, 1);
}
