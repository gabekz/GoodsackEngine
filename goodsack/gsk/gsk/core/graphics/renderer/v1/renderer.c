/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "renderer.h"

#include <stdio.h>

#include "util/filesystem.h"
#include "util/gfx.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/graphics/renderer/pipeline/pass_compute.h"
#include "core/graphics/renderer/pipeline/pipeline.h"

#include "core/graphics/lighting/lighting.h"
#include "core/graphics/lighting/skybox.h"

#include "core/graphics/ui/billboard.h"
#include "core/graphics/ui/gui_canvas.h"
#include "core/graphics/ui/gui_element.h"
#include "core/graphics/ui/gui_text.h"

#include "entity/ecs.h"

#include "core/device/device.h"
#include "core/device/device_context.h"
#include "core/drivers/vulkan/vulkan_device.h"

// gsk_Skybox test
#include "core/graphics/texture/texture.h"

#include "tools/debug/debug_context.h"
#include "tools/debug/debug_draw_line.h"

#define TESTING_DRAW_UI   1
#define TESTING_DRAW_LINE 0

gsk_Renderer *
gsk_renderer_init()
{
    int winWidth  = DEFAULT_WINDOW_WIDTH;
    int winHeight = DEFAULT_WINDOW_HEIGHT;

    gsk_Renderer *ret = malloc(sizeof(gsk_Renderer));
    GLFWwindow *window =
      /*context*/ gsk_window_create(winWidth, winHeight, &ret->vulkanDevice);

    ret->window       = window;
    ret->windowWidth  = winWidth;
    ret->windowHeight = winHeight;

    // Set Render Resolution
    ret->renderWidth =
      (RENDER_RESOLUTION_OVERRIDE) ? PSX_WIDTH : DEFAULT_WINDOW_WIDTH;
    ret->renderHeight =
      (RENDER_RESOLUTION_OVERRIDE) ? PSX_HEIGHT : DEFAULT_WINDOW_HEIGHT;

    // Create the initial scene
    gsk_Scene *scene  = malloc(sizeof(gsk_Scene));
    scene->id         = 0;
    scene->ecs        = gsk_ecs_init(ret);
    scene->has_skybox = FALSE;

    gsk_Scene **sceneList = malloc(sizeof(gsk_Scene *));
    *(sceneList)          = scene;

    ret->sceneL      = sceneList;
    ret->sceneC      = 1;
    ret->activeScene = 0;

    ret->properties = (gsk_RendererProps) {.tonemapper      = 0,
                                           .exposure        = 9.5f,
                                           .maxWhite        = 1.0f,
                                           .gamma           = 2.2f,
                                           .gammaEnable     = TRUE,
                                           .msaaEnable      = TRUE,
                                           .msaaSamples     = 4,
                                           .vignetteAmount  = 0.5f,
                                           .vignetteFalloff = 0.5f};

    ret->shadowmapOptions = (ShadowmapOptions) {
      .nearPlane = 0.00f,
      .farPlane  = 20.0f,
      .camSize   = 20.0f,

      .normalBiasMin = 0.0004f,
      .normalBiasMax = 0.0028f,
      .pcfSamples    = 4,
    };

    ret->ssaoOptions = (SsaoOptions) {
      .strength   = 1.0f,
      .bias       = 0.00035f,
      .radius     = 0.1255f,
      .kernelSize = 32,
    };

    // Ambient options
    glm_vec3_one(ret->lightOptions.ambient_color_multiplier);
    ret->lightOptions.ambient_strength   = 1.0f;
    ret->lightOptions.prefilter_strength = 1.0f;

    // Billboard test
    vec2 bbsize = {0.01f, 0.01f};
    ret->billboard =
      gsk_billboard_create(GSK_PATH("gsk://textures/gizmo/light.png"), bbsize);

    // GUI test
    gsk_Texture *guiTexture =
      texture_create(GSK_PATH("gsk://textures/gizmo/crosshair2.png"),
                     NULL,
                     (TextureOptions) {0, GL_RGBA, TRUE, TRUE});

    ret->canvas = gsk_gui_canvas_create();

    gsk_GuiElement *element = gsk_gui_element_create(
      (vec2) {1920 / 2, 1080 / 2}, (vec2) {10, 10}, guiTexture, NULL);

    // Test GUI Text
    gsk_GuiText *text = gsk_gui_text_create("Goodsack Engine");

    gsk_gui_canvas_add_element(&ret->canvas, element);
    gsk_gui_canvas_add_text(&ret->canvas, text);

    // Lighting information
    vec3 lightPos   = {-3.4f, 2.4f, 1.4f};
    vec4 lightColor = {0.73f, 0.87f, 0.91f, 1.0f};

    // Create lighting Uniform Buffer
    u32 lighting_ubo_binding = 1;
    ret->lighting_data       = gsk_lighting_initialize(lighting_ubo_binding);

    // create directional light
    gsk_lighting_add_light(
      &ret->lighting_data, (float *)lightPos, (float *)lightColor);

#if 0
    // test light
    vec3 lightPos2   = {7.0f, 6.5f, 1.0f};
    vec4 lightColor2 = {1.0f, 0.0f, 0.0f, 1.0f};
    gsk_lighting_add_light(
      &renderer->lighting_data, (float *)lightPos2, (float *)lightColor2);
#endif

    return ret;
}

gsk_ECS *
gsk_renderer_active_scene(gsk_Renderer *self, u16 sceneIndex)
{
    LOG_INFO("Loading scene: id %d", sceneIndex);
    u32 sceneCount = self->sceneC;
    if (sceneCount < sceneIndex + 1)
    {
        LOG_INFO(
          "Scene %d does not exist. Creating Scene %d", sceneIndex, sceneIndex);
        u32 newCount = sceneIndex - sceneCount + (sceneCount + 1);

        // Create a new, empty scene
        gsk_Scene *newScene  = malloc(sizeof(gsk_Scene));
        newScene->id         = newCount;
        newScene->ecs        = gsk_ecs_init(self);
        newScene->has_skybox = FALSE;

        // Update the scene list
        gsk_Scene **p              = self->sceneL;
        self->sceneL               = realloc(p, newCount * sizeof(gsk_Scene *));
        self->sceneL[newCount - 1] = newScene;
        self->sceneC               = newCount;
    }

    self->activeScene = sceneIndex;

    return self->sceneL[sceneIndex]->ecs;

    // TODO: add checks here and cleanup from previous scene for switching.
}

void
gsk_renderer_start(gsk_Renderer *renderer)
{
    // Scene initialization
    gsk_Scene *scene = renderer->sceneL[renderer->activeScene];
    gsk_ECS *ecs     = scene->ecs;

    if (GSK_DEVICE_API_OPENGL)
    {

// Create the default skybox
#if 0
        gsk_Texture *skyboxCubemap =
          texture_create_cubemap(6,
                                 "../res/textures/skybox/right.jpg",
                                 "../res/textures/skybox/left.jpg",
                                 "../res/textures/skybox/top.jpg",
                                 "../res/textures/skybox/bottom.jpg",
                                 "../res/textures/skybox/front.jpg",
                                 "../res/textures/skybox/back.jpg");

        renderer->defaultSkybox = gsk_skybox_create(skyboxCubemap);
#else
        renderer->defaultSkybox = gsk_skybox_hdr_create(texture_create_hdr(
          GSK_PATH("gsk://textures/hdr/sky_cloudy_ref.hdr")));
#endif

        // Set the current renderer skybox to that of the active scene
        renderer->activeSkybox =
          (scene->has_skybox) ? scene->skybox : renderer->defaultSkybox;

        prepass_init();

        shadowmap_init();
        // TODO: clean this up. Should be stored in UBO for directional-lights
        glm_mat4_zero(renderer->lightSpaceMatrix);
        glm_mat4_copy(shadowmap_getMatrix(), renderer->lightSpaceMatrix);

        postbuffer_init(
          renderer->renderWidth, renderer->renderHeight, &renderer->properties);

        // generate SSAO textures
        pass_ssao_init();

        // renderer->skybox = gsk_skybox_create(skyboxCubemap);

        // Send ECS event init
        gsk_ecs_event(ecs, ECS_INIT);

        // glEnable(GL_FRAMEBUFFER_SRGB);
        clearGLState();

        renderer->debugContext = gsk_debug_context_init();

        // Create camera Uniform Buffer
        u32 camera_uboId;
        u32 camera_uboSize     = sizeof(vec4) + (2 * sizeof(mat4));
        u32 camera_ubo_binding = 0;
        glGenBuffers(1, &camera_uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, camera_uboId);
        glBufferData(GL_UNIFORM_BUFFER,
                     camera_uboSize * MAX_CAMERAS,
                     NULL,
                     GL_DYNAMIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER,
                          camera_ubo_binding,
                          camera_uboId,
                          0,
                          camera_uboSize * MAX_CAMERAS);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        renderer->camera_data.uboId        = camera_uboId;
        renderer->camera_data.uboSize      = camera_uboSize;
        renderer->camera_data.totalCameras = 2; // TODO: find an alternative
        renderer->camera_data.activeCamera = 0;

    } else if (GSK_DEVICE_API_VULKAN)
    {
        gsk_ecs_event(ecs, ECS_INIT);
        // LOG_DEBUG("gsk_Renderer Start-Phase is not implemented in Vulkan");
    }
}

/* Render Functions for the pipeline */

static void
renderer_tick_OPENGL(gsk_Renderer *renderer, gsk_Scene *scene, gsk_ECS *ecs)
{
    // Settings
    glfwSwapInterval(gsk_device_getGraphicsSettings().swapInterval);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /*-------------------------------------------
        gsk_Scene Logic/Data update
    */

    gsk_device_setInput(
      gsk_device_getInput()); // TODO: Weird hack to reset for axis
    device_updateCursorState(renderer->window);
    glfwPollEvents();

    // Check fixed_update interval
    double current_time = glfwGetTime();
    if (_gsk_device_check_fixed_update(current_time))
    {
        gsk_ecs_event(ecs, ECS_INIT); // TODO: Move

        gsk_ecs_event(ecs, ECS_ON_COLLIDE);
        gsk_ecs_event(ecs, ECS_FIXED_UPDATE);

        _gsk_device_reset_fixed_update(current_time);
    }

    gsk_ecs_event(ecs, ECS_UPDATE);
    gsk_ecs_event(ecs, ECS_LATE_UPDATE);

    // Update all camera UBO's
    //__update_camera_ubo(renderer);

    /*-------------------------------------------
        Pass #0 - Depth Prepass
    */
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Pass: Depth Prepass");

    prepass_bind();
    renderer->currentPass              = DEPTH_PREPASS;
    renderer->explicitMaterial         = prepass_getMaterial();
    renderer->explicitMaterial_skinned = prepass_getMaterialSkinned();
    gsk_ecs_event(ecs, ECS_RENDER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glPopDebugGroup();
    /*-------------------------------------------
        Pass #1 - Directional Shadowmap
    */
    glPushDebugGroup(
      GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Pass: Shadowmap Depth");

    // update lighting information
    gsk_Light directional_light = renderer->lighting_data.lights[0];

#if 0
    gsk_lighting_update(
      &directional_light, directional_light.position, directional_light.color);
#else
    gsk_lighting_update(&renderer->lighting_data);
#endif

    // update shadowmap position
    shadowmap_update(directional_light.position, renderer->shadowmapOptions);
    // TODO: Move matrix storage out of renderer
    glm_mat4_zero(renderer->lightSpaceMatrix);
    glm_mat4_copy(shadowmap_getMatrix(), renderer->lightSpaceMatrix);

    // bind the shadowmap textures & framebuffers
    shadowmap_bind();

    renderer->currentPass              = SHADOW;
    renderer->explicitMaterial         = shadowmap_getMaterial();
    renderer->explicitMaterial_skinned = shadowmap_getMaterialSkinned();
    // TODO: Clean this up...
    gsk_ecs_event(ecs, ECS_RENDER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glPopDebugGroup();
    /*-------------------------------------------
        Pass #2 - SSAO Pass
    */
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 2, -1, "Pass: SSAO");
    // bind the shadowmap textures & framebuffers
    pass_ssao_bind(renderer->ssaoOptions);

    glPopDebugGroup();
    /*-------------------------------------------
        Pass #3 - Post Processing / Lighting Pass
    */
    glPushDebugGroup(
      GL_DEBUG_SOURCE_APPLICATION, 3, -1, "Pass: Lighting (Forward)");

    postbuffer_bind(renderer->properties.msaaEnable);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // binding the shadowmap to texture slot 8 (TODO:) for meshes
    shadowmap_bind_texture();

    // Bind skybox textures
    if (scene->has_skybox)
    {
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP,
                      renderer->activeSkybox->irradianceMap->id);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP,
                      renderer->activeSkybox->prefilterMap->id);

        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D,
                      renderer->activeSkybox->brdfLUTTexture->id);
    }

    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, pass_ssao_getOutputTextureId());

    // Forward-draw Event
    renderer->currentPass = REGULAR;
    gsk_ecs_event(ecs, ECS_RENDER);

    // Render skybox (NOTE: Look into whether we want to keep this in
    // the postprocessing buffer as it is now)
    if (scene->has_skybox)
    {
        glDepthFunc(GL_LEQUAL);
        gsk_skybox_draw(renderer->activeSkybox);
        glDepthFunc(GL_LESS);
    }

    glPopDebugGroup();
    /*-------------------------------------------
        Pass #4 - Final: Backbuffer draw
    */
    glPushDebugGroup(
      GL_DEBUG_SOURCE_APPLICATION, 4, -1, "Pass: Backbuffer Draw (Final)");

    postbuffer_draw(&renderer->properties);

    glPopDebugGroup();

    // Testing stuff

#if TESTING_DRAW_UI
    // vec3 pos = GLM_VEC3_ZERO_INIT;
    // gsk_billboard_draw(renderer->billboard, pos);

    gsk_gui_canvas_draw(&renderer->canvas);
#endif

#if TESTING_DRAW_LINE
#if 1
    gsk_debug_draw_line(renderer->debugContext,
                        (vec3) {-1.0f, 0, -1.0f},
                        (vec3) {-1.0f, 0, 1.0f},
                        (vec4) {1, 1, 1, 1});
    gsk_debug_draw_line(renderer->debugContext,
                        (vec3) {-1.0f, 0, -1.0f},
                        (vec3) {1.0f, 0, -1.0f},
                        (vec4) {0, 1, 0, 1});
#endif

    gsk_debug_draw_ray(renderer->debugContext,
                       (vec3) {0, 0, 0},
                       (vec3) {1, 1, 0},
                       5,
                       (vec4) {0, 1, 1, 1});

    gsk_debug_draw_ray(renderer->debugContext,
                       (vec3) {0, 0, 0},
                       (vec3) {-1, 1, 0},
                       5,
                       (vec4) {1, 0, 0, 1});

    gsk_debug_draw_ray(renderer->debugContext,
                       (vec3) {0, 0, 0},
                       (vec3) {0, 1, 1},
                       8,
                       (vec4) {1, 1, 0, 1});
#endif

// Draw debug markers
#if 1
    gsk_debug_markers_render(renderer->debugContext);
#endif

    // computebuffer_draw();
}

/*
void renderer_tick_VULKAN(gsk_Renderer *renderer, gsk_ECS *ecs) {

// Update Analytics Data

    glfwPollEvents();

    gsk_ecs_event(ecs, ECS_UPDATE);
    renderer->currentPass = REGULAR;
    gsk_ecs_event(ecs, ECS_RENDER);

    vulkan_render_draw(renderer->vulkanDevice, renderer->window);
}
*/

void
gsk_renderer_tick(gsk_Renderer *renderer)
{
    gsk_Scene *scene = renderer->sceneL[renderer->activeScene];
    gsk_ECS *ecs     = scene->ecs;

    if (GSK_DEVICE_API_OPENGL)
    {
        renderer_tick_OPENGL(renderer, scene, ecs);
    } else if (GSK_DEVICE_API_VULKAN)
    {
        // renderer_tick_VULKAN(renderer, ecs);
    }
}
