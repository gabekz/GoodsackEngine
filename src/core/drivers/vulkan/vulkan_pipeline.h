#ifndef H_VULKAN_PIPELINE
#define H_VULKAN_PIPELINE

#include <util/gfx.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _vulkanPipelineDetails VulkanPipelineDetails;

struct _vulkanPipelineDetails
{
    VkPipeline graphicsPipeline;
    VkRenderPass renderPass;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
};

VulkanPipelineDetails *
vulkan_pipeline_create(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkFormat swapchainImageFormat,
                       VkExtent2D swapchainExtent);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_PIPELINE
