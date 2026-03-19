/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_pipeline.h"

#include <stdlib.h>

#include "util/filesystem.h"
#include "util/logger.h"

#include "core/drivers/vulkan/vulkan_depth.h"
#include "core/drivers/vulkan/vulkan_descriptor.h"
#include "core/drivers/vulkan/vulkan_support.h"
#include "core/drivers/vulkan/vulkan_uniform_buffer.h"
#include "core/drivers/vulkan/vulkan_vertex_buffer.h"

#include "core/graphics/shader/shader.h"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

//#include <spirv_cross_c.h>

typedef struct SpirVBinary
{
    uint32_t *words; // SPIR-V words
    int size;        // number of words in SPIR-V binary
} SpirVBinary;

typedef struct ShaderModules
{
    VkShaderModule modules[4];
} ShaderModules;

#if 0
static void
_spv_test(SpirVBinary *p_binary)
{
    spvc_context context                = NULL;
    spvc_parsed_ir ir                   = NULL;
    spvc_compiler compiler_glsl         = NULL;
    spvc_compiler_options options       = NULL;
    spvc_resources resources            = NULL;
    const spvc_reflected_resource *list = NULL;
    const char *result                  = NULL;
    size_t count;
    size_t i;

    // Create context.
    spvc_context_create(&context);

    // Set debug callback.
    spvc_context_set_error_callback(context, error_callback, userdata);

    // Parse the SPIR-V.
    spvc_context_parse_spirv(context, p_binary->words, p_binary->size, &ir);

    // Hand it off to a compiler instance and give it ownership of the IR.
    spvc_context_create_compiler(context,
                                 SPVC_BACKEND_GLSL,
                                 ir,
                                 SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
                                 &compiler_glsl);

    // Do some basic reflection.
    spvc_compiler_create_shader_resources(compiler_glsl, &resources);
    spvc_resources_get_resource_list_for_type(
      resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);

    for (i = 0; i < count; i++)
    {
        LOG_INFO("ID: %u, BaseTypeID: %u, TypeID: %u, Name: %s\n",
                 list[i].id,
                 list[i].base_type_id,
                 list[i].type_id,
                 list[i].name);
        LOG_INFO("  Set: %u, Binding: %u\n",
                 spvc_compiler_get_decoration(
                   compiler_glsl, list[i].id, SpvDecorationDescriptorSet),
                 spvc_compiler_get_decoration(
                   compiler_glsl, list[i].id, SpvDecorationBinding));
    }

    // Modify options.
    spvc_compiler_create_compiler_options(compiler_glsl, &options);
    spvc_compiler_options_set_uint(
      options, SPVC_COMPILER_OPTION_GLSL_VERSION, 330);
    spvc_compiler_options_set_bool(
      options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_FALSE);
    spvc_compiler_install_compiler_options(compiler_glsl, options);

    spvc_compiler_compile(compiler_glsl, &result);
    LOG_INFO("Cross-compiled source: %s\n", result);

    // Frees all memory we allocated so far.
    spvc_context_destroy(context);
}
#endif

static VkShaderModule
_createShaderModule(VkDevice device, SpirVBinary *p_binary)
{
    VkShaderModuleCreateInfo createInfo = {
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = p_binary->size * sizeof(u32),
      .pCode    = (u32 *)p_binary->words,
    };

    VkShaderModule ret;
    if (vkCreateShaderModule(device, &createInfo, NULL, &ret) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create shader module!");
    }
    return ret;
}

static SpirVBinary
__create_single_shader(const char *raw_shader_code, u8 shader_type)
{
    glslang_stage_t stage =
      (shader_type == 0) ? GLSLANG_STAGE_VERTEX : GLSLANG_STAGE_FRAGMENT;

    const glslang_input_t input = {
      .language                          = GLSLANG_SOURCE_GLSL,
      .stage                             = stage,
      .client                            = GLSLANG_CLIENT_VULKAN,
      .client_version                    = GLSLANG_TARGET_VULKAN_1_3,
      .target_language                   = GLSLANG_TARGET_SPV,
      .target_language_version           = GLSLANG_TARGET_SPV_1_6,
      .code                              = raw_shader_code,
      .default_version                   = 100,
      .default_profile                   = GLSLANG_NO_PROFILE,
      .force_default_version_and_profile = FALSE,
      .forward_compatible                = FALSE,
      .messages                          = GLSLANG_MSG_DEFAULT_BIT,
      .resource                          = glslang_default_resource(),
    };

    glslang_shader_t *shader = glslang_shader_create(&input);

    SpirVBinary bin = {
      .words = NULL,
      .size  = 0,
    };

    if (!glslang_shader_preprocess(shader, &input))
    {
        // LOG_ERROR("GLSL preprocessing failed %s\n", path);
        LOG_ERROR("%s", glslang_shader_get_info_log(shader));
        LOG_ERROR("%s", glslang_shader_get_info_debug_log(shader));
        LOG_ERROR("%s", input.code);
        glslang_shader_delete(shader);
        return bin;
    }

    if (!glslang_shader_parse(shader, &input))
    {
        // LOG_ERROR("GLSL parsing failed %s\n", path);
        LOG_ERROR("%s", glslang_shader_get_info_log(shader));
        LOG_ERROR("%s", glslang_shader_get_info_debug_log(shader));
        LOG_ERROR("%s", glslang_shader_get_preprocessed_code(shader));
        glslang_shader_delete(shader);
        return bin;
    }

    glslang_program_t *program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(
          program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        // LOG_ERROR("GLSL linking failed %s\n", path);
        LOG_ERROR("%s", glslang_program_get_info_log(program));
        LOG_ERROR("%s", glslang_program_get_info_debug_log(program));
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        return bin;
    }

    glslang_program_SPIRV_generate(program, stage);

    bin.size  = glslang_program_SPIRV_get_size(program);
    bin.words = malloc(bin.size * sizeof(uint32_t));
    glslang_program_SPIRV_get(program, bin.words);

    const char *spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages) LOG_DEBUG("%s", spirv_messages);

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    return bin;
}

static ShaderModules
__create_shaders(VkDevice device, const char *path)
{
    ShaderModules ret = {0};

    int process = glslang_initialize_process();

    gsk_ShaderSource source = gsk_shader_source_parse(path, FALSE);

    SpirVBinary bin_vert = __create_single_shader(source.shaderVertex, 0);
    SpirVBinary bin_frag = __create_single_shader(source.shaderFragment, 1);

    glslang_finalize_process();

    ret.modules[0] = _createShaderModule(device, &bin_vert);
    ret.modules[1] = _createShaderModule(device, &bin_frag);

    // _spv_test(&bin_vert);

    return ret;
}

VulkanPipelineDetails *
vulkan_pipeline_create(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkFormat swapchainImageFormat,
                       VkExtent2D swapchainExtent)
{

    VulkanPipelineDetails *details = malloc(sizeof(VulkanPipelineDetails));

    ShaderModules shader_modules = __create_shaders(
      device, GSK_PATH("gsk://shaders/vulkan/std/test.shader"));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = VK_SHADER_STAGE_VERTEX_BIT,
      .module = shader_modules.modules[0],
      .pName  = "main"};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = shader_modules.modules[1],
      .pName  = "main"};

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    // Dynamic State
    VkDynamicState dynamicStates[3] = {VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_SCISSOR,
                                       VK_DYNAMIC_STATE_VERTEX_INPUT_EXT};

    VkPipelineDynamicStateCreateInfo dynamicState = {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 3,
      .pDynamicStates    = dynamicStates};

#if !(GSK_VULKAN_USING_DYNAMIC_VERTEX_INPUT)
    VkVertexInputBindingDescription bindingDescription =
      vulkan_vertex_buffer_get_binding_description();
    VkVertexInputAttributeDescription *attributeDescriptions =
      vulkan_vertex_buffer_get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions    = &bindingDescription,

      .vertexAttributeDescriptionCount = 4, // TODO: no magic
      .pVertexAttributeDescriptions    = attributeDescriptions,
    };
#endif // !(GSK_VULKAN_USING_DYNAMIC_VERTEX_INPUT)

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

    // Viewport & Scissors
    VkViewport viewport = {.x        = 0.0f,
                           .y        = 0.0f,
                           .width    = (float)swapchainExtent.width,
                           .height   = (float)swapchainExtent.height,
                           .minDepth = 0.0f,
                           .maxDepth = 1.0f};

    VkRect2D scissor = {.offset = {0, 0}, .extent = swapchainExtent};

    // Viewport State
    VkPipelineViewportStateCreateInfo viewportState = {
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports    = &viewport,
      .scissorCount  = 1,
      .pScissors     = &scissor};

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,

      .polygonMode = VK_POLYGON_MODE_FILL,
      .lineWidth   = 1.0f,

      .cullMode  = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,

      .depthBiasEnable         = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp          = 0.0f,
      .depthBiasSlopeFactor    = 0.0f};

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable   = VK_FALSE,
      .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
      .minSampleShading      = 1.0f,
      .pSampleMask           = NULL,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable      = VK_FALSE};

    // Depth Stencil State
    VkPipelineDepthStencilStateCreateInfo depthStencil = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable  = VK_TRUE,
      .depthWriteEnable = VK_TRUE,

      .depthCompareOp = VK_COMPARE_OP_LESS,

      .depthBoundsTestEnable = VK_FALSE,
      .minDepthBounds        = 0.0f, // Optional
      .maxDepthBounds        = 1.0f, // Optional

      .stencilTestEnable = VK_FALSE,
      //.front             = NULL, // Optional
      //.back              = NULL, // Optional
    };

    // Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      .blendEnable         = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
      .colorBlendOp        = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp        = VK_BLEND_OP_ADD};

    VkPipelineColorBlendStateCreateInfo colorBlending = {
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp       = VK_LOGIC_OP_COPY,
      .attachmentCount   = 1,
      .pAttachments      = &colorBlendAttachment,
      .blendConstants[0] = 0.0f,
      .blendConstants[1] = 0.0f,
      .blendConstants[2] = 0.0f,
      .blendConstants[3] = 0.0f,
    };

    // Create DescriptorSet Layout [UBO (MVP) + 1 TextureSampler Descriptors]
    details->descriptorSetLayout = vulkan_descriptor_create_layout(device);

    // Push Constants
    VkPushConstantRange psRange = {
      .offset     = 0,
      .size       = sizeof(mat4),
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };

    // Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount         = 1,
      .pSetLayouts            = &details->descriptorSetLayout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges    = &psRange,
    };

    if (vkCreatePipelineLayout(
          device, &pipelineLayoutInfo, NULL, &details->pipelineLayout) !=
        VK_SUCCESS)
    {
        LOG_ERROR("Failed to create pipeline layout!");
    }

#if !(GSK_VULKAN_USING_DYNAMIC_RENDERING)
    // Create Renderpass

    // Color Attachment
    VkAttachmentDescription colorAttachment = {
      .format  = swapchainImageFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,

      .loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,

      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    VkAttachmentReference colorAttachmentRef = {
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    // Depth Attachment
    VkAttachmentDescription depthAttachment = {
      .format         = vulkan_depth_find_format(physicalDevice),
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthAttachmentRef = {
      .attachment = 1,
      .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    // Subpass

    VkSubpassDescription subpass = {
      .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount    = 1,
      .pColorAttachments       = &colorAttachmentRef,
      .pDepthStencilAttachment = &depthAttachmentRef};

    // Render Pass (dependency)
    VkSubpassDependency dependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,

      .srcAccessMask = 0,
      .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,

      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,

      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};

    // Render Pass

    VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 2,
      .pAttachments    = attachments,
      .subpassCount    = 1,
      .pSubpasses      = &subpass,

      .dependencyCount = 1,
      .pDependencies   = &dependency};

    if (vkCreateRenderPass(
          device, &renderPassInfo, NULL, &details->renderPass) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create render pass!");
    }
#else

    VkPipelineRenderingCreateInfoKHR rendering_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      .pNext = NULL,
      .colorAttachmentCount    = 1,
      .pColorAttachmentFormats = &swapchainImageFormat,
      .depthAttachmentFormat   = vulkan_depth_find_format(physicalDevice),
      .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
    };
#endif // !(GSK_USING_DYNAMIC_RENDERING)

    // Pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {
      .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages    = shaderStages,

      .pVertexInputState = VK_NULL_HANDLE, // Disabled for dynamic input

      .pInputAssemblyState = &inputAssembly,
      .pViewportState      = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState   = &multisampling,
      .pDepthStencilState  = &depthStencil,
      .pColorBlendState    = &colorBlending,
      .pDynamicState       = &dynamicState,

      .layout     = details->pipelineLayout,
      .pNext      = &rendering_info,
      .renderPass = VK_NULL_HANDLE,

      .basePipelineHandle = VK_NULL_HANDLE, // Optional
      .basePipelineIndex  = -1,             // Optional
    };

    if (vkCreateGraphicsPipelines(device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipelineInfo,
                                  NULL,
                                  &details->graphicsPipeline) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create graphics pipeline!");
    }

    return details;
}
