/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_render.h"

#include "util/filesystem.h"
#include "util/gfx.h"

#include "core/drivers/vulkan/vulkan.h"
#include "core/drivers/vulkan/vulkan_command.h"
#include "core/drivers/vulkan/vulkan_descriptor.h"
#include "core/drivers/vulkan/vulkan_device.h"
#include "core/drivers/vulkan/vulkan_support.h"

#include "core/graphics/texture/texture.h"

void
vulkan_render_setup(VulkanDeviceContext *context)
{
    // Create a Command Pool
    LOG_DEBUG("Create Command Pool");
    context->commandPool =
      vulkan_command_pool_create(context->physicalDevice, context->device);
    // Create Command Buffers
    LOG_DEBUG("Create command buffers");
    context->commandBuffers =
      vulkan_command_buffer_create(context->device, context->commandPool);

    // Create UNIFORM BUFFERS
    LOG_DEBUG("Create uniform buffers");
    vulkan_uniform_buffer_create(context->physicalDevice,
                                 context->device,
                                 &context->uniformBuffers,
                                 &context->uniformBuffersMemory,
                                 &context->uniformBuffersMapped);

    // Create Descriptor Pool
    LOG_DEBUG("Create descriptor pool");
    context->descriptorPool = vulkan_descriptor_pool_create(context->device);

    // Create a texture
    LOG_DEBUG("Create a test texture");
    gsk_Texture *texture =
      texture_create(GSK_PATH("gsk://textures/prototype/uv_checker.png"),
                     context,
                     (TextureOptions) {16, GL_SRGB_ALPHA, true, true});

    // Create Descriptor Sets
    LOG_DEBUG("Create descriptor sets");
    context->descriptorSets = vulkan_descriptor_sets_create(
      context->device,
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

static void
vulkan_render_record_begin(VulkanDeviceContext *context,
                           u32 imageIndex,
                           VkCommandBuffer *commandBuffer)
{
    vulkan_image_memory_barrier(
      context->device,
      commandBuffer,
      context->commandPool,
      context->graphicsQueue,
      context->swapChainDetails->swapchainImages[imageIndex],
      context->swapChainDetails->swapchainImageFormat,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkClearValue clearColor   = {{{0.0f, 0.1f, 0.2f, 1.0f}}};
    VkClearValue depthStencil = {1.0f, 0.0f};

    VkClearValue clearValues[] = {clearColor, depthStencil};

#if !(GSK_VULKAN_USING_DYNAMIC_RENDERING)
    VkRenderPassBeginInfo renderPassInfo = {
      .sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = context->pipelineDetails->renderPass,
      .framebuffer =
        context->swapChainDetails->swapchainFramebuffers[imageIndex],
      .renderArea.offset = {0, 0},
      .renderArea.extent = context->swapChainDetails->swapchainExtent,

      .clearValueCount = 2,
      .pClearValues    = clearValues};

    vkCmdBeginRenderPass(
      *commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
#else

    VkRenderingAttachmentInfoKHR color_attachment = {
      .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
      .pNext       = NULL,
      .imageView   = context->swapChainDetails->swapchainImageViews[imageIndex],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .resolveMode = VK_RESOLVE_MODE_NONE,
      .resolveImageView   = VK_NULL_HANDLE,
      .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue         = clearColor,
    };

    VkRenderingAttachmentInfoKHR depth_attachment = {
      .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
      .pNext              = NULL,
      .imageView          = context->depthResources->depthImageView,
      .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .resolveMode        = VK_RESOLVE_MODE_NONE,
      .resolveImageView   = VK_NULL_HANDLE,
      .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue         = depthStencil,
    };

    VkRenderingInfoKHR rendering_info = {
      .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
      .renderArea.offset    = {0, 0},
      .renderArea.extent    = context->swapChainDetails->swapchainExtent,
      .layerCount           = 1,
      .viewMask             = 0,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &color_attachment,
      .pDepthAttachment     = &depth_attachment,
    };

    vkCmdBeginRendering(*commandBuffer, &rendering_info);
#endif // !(GSK_VULKAN_USING_DYNAMIC_RENDERING)

    // Bind pipeline (containing loaded shader modules)
    vkCmdBindPipeline(*commandBuffer,
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      context->pipelineDetails->graphicsPipeline);

    // Set viewports and scissors (again?)
    // TODO: viewport and scissor specified per abstracted render pass
    // TODO: BindPipleine (GraphicsPipeline) for the appropriate model (create
    // with variances i.e, different vertexInputs, culling etc.)
    // One "abstract" render pipeline can have mulitple graphics pipelines
    VkViewport viewport = {
      .x        = 0.0f,
      .y        = 0.0f,
      .width    = (float)context->swapChainDetails->swapchainExtent.width,
      .height   = (float)context->swapChainDetails->swapchainExtent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
    vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {.offset = {0, 0},
                        .extent = context->swapChainDetails->swapchainExtent};
    vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(*commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            context->pipelineDetails->pipelineLayout,
                            0,
                            1,
                            &context->descriptorSets[context->currentFrame],
                            0,
                            NULL);

#if 0
#if GSK_VULKAN_USING_DYNAMIC_VERTEX_INPUT
    VkVertexInputBindingDescription2EXT vertex_binding_description_ext = {
      .sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
      .binding   = 0,
      .stride    = (3 + 2 + 3 + 3) * sizeof(f32),
      .divisor   = 1,
    };

    VkVertexInputAttributeDescription2EXT vertex_attribute_description_ext[4] =
      {
        {
          .sType   = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
          .binding = 0,
          .location = 0,
          .format   = VK_FORMAT_R32G32B32_SFLOAT,
          .offset   = 0,
        },
        {
          .sType   = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
          .binding = 0,
          .location = 1,
          .format   = VK_FORMAT_R32G32B32_SFLOAT,
          .offset   = 3 * sizeof(f32),
        },
        {
          .sType   = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
          .binding = 0,
          .location = 2,
          .format   = VK_FORMAT_R32G32B32_SFLOAT,
          .offset   = 5 * sizeof(f32),
        },
        {
          .sType   = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
          .binding = 0,
          .location = 3,
          .format   = VK_FORMAT_R32G32B32_SFLOAT,
          .offset   = 8 * sizeof(f32),
        },
      };

    context->pfnCmdSetVertexInputEXT(*commandBuffer,
                                     1,
                                     &vertex_binding_description_ext,
                                     4,
                                     vertex_attribute_description_ext);
#endif // GSK_VULKAN_USING_DYNAMIC_VERTEX_INPUT
#endif
}

static void
vulkan_render_record_end(VulkanDeviceContext *context,
                         u32 imageIndex,
                         VkCommandBuffer *commandBuffer)
{
    // vkCmdEndRenderPass(*commandBuffer);
    vkCmdEndRendering(*commandBuffer);

    vulkan_image_memory_barrier(
      context->device,
      commandBuffer,
      context->commandPool,
      context->graphicsQueue,
      context->swapChainDetails->swapchainImages[imageIndex],
      context->swapChainDetails->swapchainImageFormat,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

// ------------------------ DRAW ------------------------------------- //

void
vulkan_render_draw_begin(VulkanDeviceContext *context, GLFWwindow *window)
{
    VK_CHECK(vkWaitForFences(context->device,
                             1,
                             &context->inFlightFences[context->currentFrame],
                             VK_TRUE,
                             UINT64_MAX));

    VkResult result = vkAcquireNextImageKHR(
      context->device,
      context->swapChainDetails->swapchain,
      UINT64_MAX,
      context->imageAvailableSemaphores[context->currentFrame],
      VK_NULL_HANDLE,
      &context->presentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        LOG_DEBUG("VK_ERROR_OUT_OF_DATE_KHR - recreating swapchain");
        context->swapChainDetails =
          vulkan_swapchain_recreate(context->physicalDevice,
                                    context->device,
                                    context->swapChainDetails,
                                    context->surface,
                                    context->pipelineDetails->renderPass,
                                    &context->depthResources,
                                    window // TODO: Maybe don't use this??
          );
        return; // must pull-out for requeue
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOG_ERROR("Failed to acquire next image!");
    }

    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      //.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .flags            = 0,
      .pInheritanceInfo = NULL, // Optional
    };

    if (vkBeginCommandBuffer(context->commandBuffers[context->currentFrame],
                             &beginInfo) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to begin recording command buffer!");
    }

    // Record the command buffer
    vulkan_render_record_begin(context,
                               context->presentImageIndex,
                               &context->commandBuffers[context->currentFrame]);
}

void
vulkan_render_draw_end(VulkanDeviceContext *context, GLFWwindow *window)
{

    // End recording
    vulkan_render_record_end(context,
                             context->presentImageIndex,
                             &context->commandBuffers[context->currentFrame]);

    // end command buffer
    if (vkEndCommandBuffer(context->commandBuffers[context->currentFrame]) !=
        VK_SUCCESS)
    {
        LOG_ERROR("Failed to record command buffer!");
    }

    // Must be done AFTER we potentially recreate the swapchain.
    // Avoids Fence deadlock.
    VK_CHECK(vkResetFences(
      context->device, 1, &context->inFlightFences[context->currentFrame]));

    /*
    // Reset before recording
    VK_CHECK(vkResetCommandBuffer(
                context->commandBuffers[context->currentFrame], 0));
    */

    VkSemaphore waitSemaphores[] = {
      context->imageAvailableSemaphores[context->currentFrame]};
    VkSemaphore signalSemaphores[] = {
      context->renderFinishedSemaphores[context->currentFrame]};
    VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    // Update UBO data
    // LOG_DEBUG("update uniform buffers");
    vulkan_uniform_buffer_update(context->currentFrame,
                                 context->uniformBuffersMapped,
                                 context->swapChainDetails->swapchainExtent);

    VkSubmitInfo submitInfo = {
      .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = waitSemaphores,
      .pWaitDstStageMask  = waitStages,

      .commandBufferCount = 1,
      .pCommandBuffers    = &context->commandBuffers[context->currentFrame],

      .signalSemaphoreCount = 1,
      .pSignalSemaphores    = signalSemaphores};

    VK_CHECK(vkQueueSubmit(context->graphicsQueue,
                           1,
                           &submitInfo,
                           context->inFlightFences[context->currentFrame]));

    VkSwapchainKHR swapChains[] = {context->swapChainDetails->swapchain};

    VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = signalSemaphores,

      .swapchainCount = 1,
      .pSwapchains    = swapChains,
      .pImageIndices  = &context->presentImageIndex,

      .pResults = NULL // Optional
    };

    vkQueuePresentKHR(context->graphicsQueue, &presentInfo);
    context->currentFrame = (context->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
