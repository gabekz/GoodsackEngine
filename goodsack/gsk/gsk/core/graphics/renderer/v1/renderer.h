/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "util/gfx.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/graphics/lighting/lighting.h"
#include "core/graphics/lighting/skybox.h"
#include "core/graphics/material/material.h"
#include "core/graphics/scene/scene.h"
#include "core/graphics/ui/billboard.h"
#include "core/graphics/ui/gui_element.h"
#include "core/graphics/ui/gui_text.h"

#include "core/graphics/renderer/renderer_props.inl"

#include "core/graphics/renderer/pipeline/pass_shadowmap.h"
#include "core/graphics/renderer/pipeline/pass_ssao.h"

#include "core/drivers/vulkan/vulkan_device.h"

#include "tools/debug/debug_context.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RENDER_RESOLUTION_OVERRIDE SYS_DISABLED
#define PSX_WIDTH                  320
#define PSX_HEIGHT                 240

#define MAX_CAMERAS 4

typedef enum renderPass { REGULAR = 0, DEPTH_PREPASS, SHADOW } RenderPass;

typedef struct _renderer Renderer;

struct _renderer
{
    GLFWwindow *window;
    RendererProps properties;      // Frame properties/configuration
    int windowWidth, windowHeight; // window resolution
    int renderWidth, renderHeight; // render resolution

    gsk_Scene **sceneL;
    ui16 sceneC, activeScene;

    RenderPass currentPass; // TODO: rename -> RenderStage
    gsk_Material *explicitMaterial;

    gsk_Billboard2D *billboard; // Billboard testing
    gsk_GuiElement *uiImage;    // GuiElement test
    gsk_GuiText *uiText;        // GuiText test

    gsk_Skybox *activeSkybox;  // Active skybox that is being rendered
    gsk_Skybox *defaultSkybox; // Default skybox set for each scene on creation

    // Hacky shit for temporary shadowmap values
    gsk_ShaderProgram *shaderDepthMap;
    gsk_Material *materialDepthMap;
    ui32 depthMapFBO;
    ui32 depthMapTexture;
    mat4 lightSpaceMatrix;

    ui32 drawCalls;
    ui32 faces;
    ui32 totalVertices;

    // TODO: Fix this shit as well.
    gsk_Light *light;
    ShadowmapOptions shadowmapOptions;
    SsaoOptions ssaoOptions;

    struct
    {
        vec3 ambient_color_multiplier;
        float ambient_strength, prefilter_strength;
    } lightOptions;

    // TODO: still hacky shit
    VulkanDeviceContext *vulkanDevice;
    ui32 hdrTextureId;

    DebugContext *debugContext;

    // Camera information
    struct
    {
        ui32 uboId, uboSize;
        // CameraData **cameras; // List of cameras

        ui32 totalCameras; // TODO: find an alternative
        ui32 activeCamera;
    } camera_data;
};

/**
 * Initialize the Renderer.
 * @return allocated Renderer structure
 */
Renderer *
renderer_init();

// Rendering Loop
void
renderer_start(Renderer *renderer);
void
renderer_tick(Renderer *renderer);

/* scene management */

/**
 * Sets the active scene for the renderer. Will create a new scene
 * if the specified index does not yet exist.
 *
 * @param[in] self Pointer to the renderer
 * @param[in] sceneIndex Index of the scene to load/create
 * @return Pointer to the ECS struct owned by the scene
 */
struct _ecs *
renderer_active_scene(Renderer *self, ui16 sceneIndex);
//-------------------------------

// #endif // __cplusplus

#ifdef __cplusplus
}
#endif

#endif // __RENDERER_H__
