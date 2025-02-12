/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
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
#include "core/graphics/ui/gui_canvas.h"
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
#define PSX_WIDTH                  640
#define PSX_HEIGHT                 480

#define MAX_CAMERAS 4
//#define MAX_LIGHTS  64

#define RENDERER_UBO_BINDING_CAMERA   0
#define RENDERER_UBO_BINDING_LIGHTING 1

typedef enum GskRenderPass_ {
    GskRenderPass_GBuffer = 0,
    GskRenderPass_Shadowmap,
    GskRenderPass_Lighting,
    GskRenderPass_Skybox,
} GskRenderPass_;

typedef u32 GskRenderPass;

typedef struct gsk_Renderer
{
    GLFWwindow *window;
    gsk_RendererProps properties;  // Frame properties/configuration
    int windowWidth, windowHeight; // window resolution
    int renderWidth, renderHeight; // render resolution
    f32 window_aspect_ratio;       // window aspect ratio

    gsk_Scene **sceneL;
    u16 sceneC, activeScene, scene_queue_index;

    GskRenderPass currentPass; // TODO: rename -> RenderStage
    gsk_Material *explicitMaterial;
    gsk_Material
      *explicitMaterial_skinned; // skinned version of explicit material

    gsk_Billboard2D *billboard; // Billboard testing
    gsk_GuiCanvas canvas;       // Canvas test

    gsk_Skybox *activeSkybox;  // Active skybox that is being rendered
    gsk_Skybox *defaultSkybox; // Default skybox set for each scene on creation

    // Hacky shit for temporary shadowmap values
    gsk_ShaderProgram *shaderDepthMap;
    gsk_Material *materialDepthMap;
    mat4 lightSpaceMatrix;

    // Options
    ShadowmapOptions shadowmapOptions;
    SsaoOptions ssaoOptions;
    struct
    {
        vec3 ambient_color_multiplier;
        float ambient_strength, prefilter_strength;
    } lightOptions;

    // gsk_LightingData lighting_data;

    // Camera information
    struct
    {
        u32 uboId, uboSize;
        // CameraData **cameras; // List of cameras

        u32 totalCameras; // TODO: find an alternative
        u32 activeCamera;
    } camera_data;

    // TODO: still hacky shit
    VulkanDeviceContext *vulkanDevice;
    gsk_DebugContext *debugContext;

    // TODO: remove - testing opengl samplers
    u32 sampler0_id;
    u32 sampler1_id;

} gsk_Renderer;

/**
 * Initialize the Renderer.
 * @return allocated Renderer structure
 */
gsk_Renderer *
gsk_renderer_init(const char *app_name);

/**
 * Sets the active scene for the renderer. Will create a new scene
 * if the specified index does not yet exist.
 *
 * @param[in] self Pointer to the renderer
 * @param[in] sceneIndex Index of the scene to load/create
 * @return Pointer to the gsk_ECS struct owned by the scene
 */
struct gsk_ECS *
gsk_renderer_active_scene(gsk_Renderer *self, u16 sceneIndex);

// Rendering Loop
void
gsk_renderer_start(gsk_Renderer *renderer);
void
gsk_renderer_tick(gsk_Renderer *renderer);

/**
 * Utility function to handle window resizing
 */
void
gsk_renderer_resize(gsk_Renderer *p_self, int new_width, int new_height);

//-------------------------------

// #endif // __cplusplus

#ifdef __cplusplus
}
#endif

#endif // __RENDERER_H__
