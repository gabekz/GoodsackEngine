#include "vulkan_pipeline.h"

#include <stdlib.h>
#include <util/logging.h>

struct FileDescriptor {
    char *buffer;
    long filelen;
};

static struct FileDescriptor *_parseShader(const char *path) {
    FILE *fileptr;
    char *buffer;
    long filelen;
    
    fileptr = fopen(path, "rb");          // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file
    
    buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
    fread(buffer, filelen, 1, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file

    // LOG_DEBUG("Contents: %s", buffer);
    // LOG_DEBUG("Filelen: %d", filelen);

    struct FileDescriptor *ret = malloc(sizeof(struct FileDescriptor));
    ret->buffer = buffer;
    ret->filelen = filelen;

    return ret;
}

static VkShaderModule _createShaderModule(VkDevice device, const char *path) {

    struct FileDescriptor *file = _parseShader(path);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = file->filelen;
    createInfo.pCode = (uint32_t *)file->buffer;

    VkShaderModule ret;
    if(vkCreateShaderModule(device, &createInfo, NULL, &ret) != VK_SUCCESS) {
        LOG_ERROR("Failed to create shader module!");
        return NULL;
    }
    return ret;
}

VulkanPipelineDetails *vulkan_pipeline_create(VkDevice device,
    VkFormat swapchainImageFormat, VkExtent2D swapchainExtent) {

    VulkanPipelineDetails *details = malloc(sizeof(VulkanPipelineDetails));

    VkShaderModule vertShaderModule = _createShaderModule(device,
            "../res/shaders/vulkan/vert.spv");
    VkShaderModule fragShaderModule = _createShaderModule(device,
            "../res/shaders/vulkan/frag.spv");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main" 
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo };
    
// Dynamic State
    VkDynamicState dynamicStates[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates
    };

// Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = NULL,

        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = NULL
    };

// Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

// Viewport & Scissors
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)swapchainExtent.width,
        .height = (float)swapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapchainExtent
    };

    // Viewport State
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

// Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,

        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,

        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,

        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f
    };

// Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

// Depth & Stencil Testing
    VkPipelineDepthStencilStateCreateInfo depthStencil;

// Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f,
    };

// Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL,
    };

    if(vkCreatePipelineLayout(device, &pipelineLayoutInfo,
            NULL, &details->pipelineLayout) != VK_SUCCESS) {
        LOG_ERROR("Failed to create pipeline layout!");
    }

// Create Renderpass
    VkAttachmentDescription colorAttachment = {
        .format = swapchainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,

        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,

        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };

    // Render Pass (dependency)
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,

        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,

        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    // Render Pass
    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,

        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    if(vkCreateRenderPass(device, &renderPassInfo, NULL, &details->renderPass) != VK_SUCCESS) {
        LOG_ERROR("Failed to create render pass!");
    }

// Pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,

        .pVertexInputState      = &vertexInputInfo,
        .pInputAssemblyState    = &inputAssembly,
        .pViewportState         = &viewportState,
        .pRasterizationState    = &rasterizer,
        .pMultisampleState      = &multisampling,
        .pDepthStencilState     = NULL, // Optional
        .pColorBlendState       = &colorBlending,
        .pDynamicState          = &dynamicState,

        .layout = details->pipelineLayout,
        .renderPass = details->renderPass,
        .subpass = 0,

        .basePipelineHandle = VK_NULL_HANDLE, // Optional
        .basePipelineIndex = -1, // Optional
    };

    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
            &pipelineInfo, NULL, &details->graphicsPipeline) != VK_SUCCESS) {
        LOG_ERROR("Failed to create graphics pipeline!");
    }

    return details;
}
