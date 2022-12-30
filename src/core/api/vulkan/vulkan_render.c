#include "vulkan_render.h"

#include <util/gfx.h>

#include <core/api/vulkan/vulkan_device.h>
#include <core/api/vulkan/vulkan_command.h>
#include <core/api/vulkan/vulkan_support.h>

#include <model/primitives.h>

#define TEST_RENDER_PRIMITIVE   0   // 0 - Model Loader | 1 - Primitive
#define TEST_RENDER_MODE        0   // 0 - VkCmdDraw    | 1 - VkCmdDrawIndexed

void vulkan_render_setup(VulkanDeviceContext *context) {
// Create a Command Pool
    LOG_DEBUG("Create Command Pool");
    context->commandPool =
        vulkan_command_pool_create(context->physicalDevice, context->device);
// Create Command Buffers
    LOG_DEBUG("Create command buffers");
    context->commandBuffers = 
        vulkan_command_buffer_create(context->device, context->commandPool);

/*
// Create a VERTEX BUFFER
    LOG_DEBUG("Create vertex buffer");

#if TEST_RENDER_PRIMITIVE == 0

#elif TEST_RENDER_PRIMITIVE == 1

    float *vertices = PRIM_ARR_V_PYRAMID;
    int size = PRIM_SIZ_V_PYRAMID * sizeof(float);

#endif

    VulkanVertexBuffer *vb = 
        vulkan_vertex_buffer_create(
                context->physicalDevice,
                context->device, 
                context->graphicsQueue,
                context->commandPool,
                vertices,
                size);

    context->vertexBuffer = vb;

#if TEST_RENDER_MODE == 1

    ui16 *indices = PRIM_ARR_I_PYRAMID;
    ui32 indicesCount = PRIM_SIZ_I_PYRAMID;

    context->indexBuffer = *vulkan_index_buffer_create(
            context->physicalDevice,
            context->device,
            context->commandPool,
            context->graphicsQueue,
            indices, indicesCount
    );

#endif
*/

// Create UNIFORM BUFFERS
    LOG_DEBUG("Create uniform buffers");
    vulkan_uniform_buffer_create(context->physicalDevice, context->device,
           &context->uniformBuffers, &context->uniformBuffersMemory,
           &context->uniformBuffersMapped);

// Create Descriptor Pool
    LOG_DEBUG("Create descriptor pool");
    context->descriptorPool = vulkan_descriptor_pool_create(context->device);

// Create a texture
    LOG_DEBUG("Create a test texture");
    Texture *texture = 
        texture_create("../res/textures/pbr/cerberus/Cerberus_A.tga",
                0, 0, 0, context);

// Create Descriptor Sets
    LOG_DEBUG("Create descriptor sets");
    context->descriptorSets =
        vulkan_descriptor_sets_create(context->device,
                context->descriptorPool,
                context->pipelineDetails->descriptorSetLayout,
                // UBO
                context->uniformBuffers, // 1 UBO per FLIGHT
                sizeof(UniformBufferObject),
                // Texture Sampler
                texture->vulkan.textureImageView,
                texture->vulkan.textureSampler);
}

// ------------------------ RECORD ----------------------------------- //

static void vulkan_render_record_begin(VulkanDeviceContext *context, ui32 imageIndex,
        VkCommandBuffer *commandBuffer)
{
    //LOG_DEBUG("RECORDING command buffer");

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        //.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .flags = 0,
        .pInheritanceInfo = NULL, // Optional
    };
    
    if (vkBeginCommandBuffer(*commandBuffer, &beginInfo) != VK_SUCCESS) {
        LOG_ERROR("Failed to begin recording command buffer!");
    }

    VkClearValue clearColor = {{{0.0f, 0.1f, 0.2f, 1.0f}}};
    VkClearValue depthStencil = {1.0f, 0.0f};

    VkClearValue clearValues[] = {
        clearColor,
        depthStencil
    };

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context->pipelineDetails->renderPass,
        .framebuffer = context->swapChainDetails->swapchainFramebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = context->swapChainDetails->swapchainExtent,

        .clearValueCount = 2,
        .pClearValues = clearValues
    };

    vkCmdBeginRenderPass(*commandBuffer,
        &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline (containing loaded shader modules)
    vkCmdBindPipeline(*commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        context->pipelineDetails->graphicsPipeline);

    // Set viewports and scissors (again?)
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)context->swapChainDetails->swapchainExtent.width,
        .height = (float)context->swapChainDetails->swapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = context->swapChainDetails->swapchainExtent
    };
    vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);

    //LOG_DEBUG("Binding Vertex Buffer");
    //VkBuffer vertexBuffers[] = {context->vertexBuffer->buffer};
    VkDeviceSize offsets[] = {0};

    //VkBuffer indexBuffer = context->indexBuffer.buffer;

    //vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            context->pipelineDetails->pipelineLayout, 0, 1,
            &context->descriptorSets[context->currentFrame], 0, NULL);

#if TEST_RENDER_MODE == 0

    //vkCmdDraw(*commandBuffer, context->vertexBuffer->size, 1, 0, 0);

#elif TEST_RENDER_MODE == 1

    //vkCmdBindIndexBuffer(*commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    //vkCmdDrawIndexed(*commandBuffer, context->indexBuffer.indicesCount,
    //        1, 0, 0, 0);
#endif

}

static void vulkan_render_record_end(VkCommandBuffer *commandBuffer) {
    vkCmdEndRenderPass(*commandBuffer);

    if (vkEndCommandBuffer(*commandBuffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to record command buffer!");
    }
}

// ------------------------ DRAW ------------------------------------- //

void vulkan_render_draw_begin(VulkanDeviceContext *context, GLFWwindow *window) {
    VK_CHECK(vkWaitForFences(context->device, 1,
                &context->inFlightFences[context->currentFrame],
                VK_TRUE, UINT64_MAX));

    VkResult result = vkAcquireNextImageKHR(context->device,
            context->swapChainDetails->swapchain, UINT64_MAX,
            context->imageAvailableSemaphores[context->currentFrame],
            VK_NULL_HANDLE, &context->presentImageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        LOG_DEBUG("VK_ERROR_OUT_OF_DATE_KHR - recreating swapchain");
        context->swapChainDetails = vulkan_swapchain_recreate(
                context->physicalDevice,
                context->device,
                context->swapChainDetails,
                context->surface,
                context->pipelineDetails->renderPass,
                &context->depthResources,
                window // TODO: Maybe don't use this??
        );
        return; // must pull-out for requeue
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR("Failed to acquire next image!");
    }

    // Record the command buffer
    vulkan_render_record_begin(context, context->presentImageIndex,
            &context->commandBuffers[context->currentFrame]);
}

void vulkan_render_draw_end(VulkanDeviceContext *context, GLFWwindow *window) {

    // End recording
    vulkan_render_record_end(&context->commandBuffers[context->currentFrame]);

    // Must be done AFTER we potentially recreate the swapchain.
    // Avoids Fence deadlock.
    VK_CHECK(vkResetFences(context->device, 1,
            &context->inFlightFences[context->currentFrame]));

    /*
    // Reset before recording
    VK_CHECK(vkResetCommandBuffer(
                context->commandBuffers[context->currentFrame], 0));
    */

    VkSemaphore waitSemaphores[] =
        {context->imageAvailableSemaphores[context->currentFrame]};
    VkSemaphore signalSemaphores[] = 
        {context->renderFinishedSemaphores[context->currentFrame]};
    VkPipelineStageFlags waitStages[] =
        {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    // Update UBO data
    //LOG_DEBUG("update uniform buffers");
    /*
    vulkan_uniform_buffer_update(context->currentFrame,
            context->uniformBuffersMapped,
            context->swapChainDetails->swapchainExtent);
    */

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,

        .commandBufferCount = 1,
        .pCommandBuffers = &context->commandBuffers[context->currentFrame],

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    VK_CHECK(vkQueueSubmit(context->graphicsQueue, 1,
                &submitInfo, context->inFlightFences[context->currentFrame]));

    VkSwapchainKHR swapChains[] = {context->swapChainDetails->swapchain};

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,

        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &context->presentImageIndex,

        .pResults = NULL // Optional
    };

    vkQueuePresentKHR(context->graphicsQueue, &presentInfo);
    context->currentFrame = (context->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
