/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "renderer.h"

#include <stdio.h>

#include "util/filesystem.h"
#include "util/gfx.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/graphics/renderer/pipeline/pass_bloom.h"
#include "core/graphics/renderer/pipeline/pass_compute.h"
#include "core/graphics/renderer/pipeline/pipeline.h"

#include "core/graphics/lighting/lighting.h"
#include "core/graphics/lighting/skybox.h"

#include "core/graphics/particles/particle_system.h"
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

#define TESTING_DRAW_UI           1
#define TESTING_DRAW_LINE         0
#define TESTING_GLSAMPLER_OBJECTS 0
#define LIGHTING_CULL_GLOBAL      1

gsk_Renderer *
gsk_renderer_init(const char *app_name)
{
    int winWidth             = DEFAULT_WINDOW_WIDTH;
    int winHeight            = DEFAULT_WINDOW_HEIGHT;
    const char *winImagePath = GSK_PATH("gsk://textures/defaults/missing.jpg");

    gsk_Renderer *ret = malloc(sizeof(gsk_Renderer));
    GLFWwindow *window =
      /*context*/ gsk_window_create(
        winWidth, winHeight, winImagePath, app_name, &ret->vulkanDevice);

    ret->window              = window;
    ret->windowWidth         = winWidth;
    ret->windowHeight        = winHeight;
    ret->window_aspect_ratio = (f32)winWidth / (f32)winHeight;
    ret->p_prev_material     = NULL;

    // Set Render Resolution
    ret->renderWidth  = (RENDER_RESOLUTION_OVERRIDE) ? PSX_WIDTH : winWidth;
    ret->renderHeight = (RENDER_RESOLUTION_OVERRIDE) ? PSX_HEIGHT : winHeight;

    // Create the initial scene
    gsk_Scene *scene  = malloc(sizeof(gsk_Scene));
    scene->id         = 0;
    scene->ecs        = gsk_ecs_init(ret);
    scene->has_skybox = FALSE;

    // Fog options
    // TODO: Don't initialize the first fog options here
    scene->fogOptions.fog_start   = -1.0f;
    scene->fogOptions.fog_end     = 100.0f;
    scene->fogOptions.fog_density = 0.1f;
    {
        vec4 fogcol4 = DEFAULT_CLEAR_COLOR;
        glm_vec3_copy(fogcol4, scene->fogOptions.fog_color);
    }

    // FIRST Scene Lighting
    // TODO: Don't initialize the first scene lighting data here
    scene->lighting_data =
      gsk_lighting_initialize(RENDERER_UBO_BINDING_LIGHTING);

    vec3 lightPos   = {-3.4f, 2.4f, 1.4f};
    vec4 lightColor = {0.73f, 0.87f, 0.91f, 1.0f};

    // create directional light
    gsk_lighting_add_light(
      &scene->lighting_data, (float *)lightPos, (float *)lightColor);

    // Scene list

    gsk_Scene **sceneList = malloc(sizeof(gsk_Scene *));
    *(sceneList)          = scene;

    ret->sceneL            = sceneList;
    ret->sceneC            = 1;
    ret->activeScene       = 0;
    ret->scene_queue_index = 0;

    ret->defaultSkybox = NULL;

    ret->properties = (gsk_RendererProps) {
      .tonemapper      = 3,
      .exposure        = 9.5f,
      .maxWhite        = 1.0f,
      .gamma           = 2.2f,
      .gammaEnable     = TRUE,
      .msaaEnable      = TRUE,
      .msaaSamples     = 16,
      .vignetteAmount  = 0.5f,
      .vignetteFalloff = 0.5f,
      .vignetteColor   = {0, 0, 0},

      .bloom_intensity = 0.3f,
      .bloom_radius    = 0.01f,
      .bloom_threshold = 0.0f,
    };

    ret->shadowmapOptions = (ShadowmapOptions) {
      .nearPlane = 0.00f,
      .farPlane  = 40.0f,
      .camSize   = 20.0f,

      .normalBiasMin = 0.0004f,
      .normalBiasMax = 0.0028f,
      .pcfSamples    = 4,
    };

    ret->ssaoOptions = (SsaoOptions) {
      .strength   = 2.5f,
      .bias       = 0.285f,
      .radius     = 0.45f,
      .kernelSize = 64,
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

    gsk_GuiElement *element =
      gsk_gui_element_create(GskGuiElementAnchorType_Center,
                             (vec2) {0, 0},
                             (vec2) {10, 10},
                             (vec3) {1, 1, 1},
                             guiTexture,
                             NULL);

    gsk_gui_canvas_add_element(&ret->canvas, element);

#if 0
    // test light
    vec3 lightPos2   = {7.0f, 6.5f, 1.0f};
    vec4 lightColor2 = {1.0f, 0.0f, 0.0f, 1.0f};
    gsk_lighting_add_light(
      &renderer->lighting_data, (float *)lightPos2, (float *)lightColor2);
#endif

    // computebuffer_init();

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
        // TODO: Create new empty scenes for all newly "sandwiched" scenes.
        gsk_Scene *newScene  = malloc(sizeof(gsk_Scene));
        newScene->id         = newCount;
        newScene->ecs        = gsk_ecs_init(self);
        newScene->has_skybox = FALSE;

        // Fog options
        newScene->fogOptions.fog_start   = -1.0f;
        newScene->fogOptions.fog_end     = 100.0f;
        newScene->fogOptions.fog_density = 0.1f;
        {
            vec4 fogcol4 = DEFAULT_CLEAR_COLOR;
            glm_vec3_copy(fogcol4, newScene->fogOptions.fog_color);
        }

        // Scene Lighting

        newScene->lighting_data =
          gsk_lighting_initialize(RENDERER_UBO_BINDING_LIGHTING);

        vec3 lightPos   = {-3.4f, 2.4f, 1.4f};
        vec4 lightColor = {0.73f, 0.87f, 0.91f, 1.0f};

        // create directional light
        gsk_lighting_add_light(
          &newScene->lighting_data, (float *)lightPos, (float *)lightColor);

        // Update the scene list
        gsk_Scene **p              = self->sceneL;
        self->sceneL               = realloc(p, newCount * sizeof(gsk_Scene *));
        self->sceneL[newCount - 1] = newScene;
        self->sceneC               = newCount;
    }

    self->activeScene       = sceneIndex;
    self->scene_queue_index = sceneIndex;

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
        if (renderer->defaultSkybox == NULL)
        {

            renderer->defaultSkybox = gsk_skybox_hdr_create(texture_create_hdr(
              GSK_PATH("gsk://textures/hdr/sky_cloudy_ref.hdr")));
        }
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

        // init bloom
        pass_bloom_init();

        // setup particle system
        gsk_particle_system_initialize();

        // renderer->skybox = gsk_skybox_create(skyboxCubemap);

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

// Testing for sampler objects
#if TESTING_GLSAMPLER_OBJECTS
        // sampler0
        glGenSamplers(1, &renderer->sampler0_id);
        glSamplerParameteri(renderer->sampler0_id,
                            GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(
          renderer->sampler0_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameterf(
          renderer->sampler0_id, GL_TEXTURE_MAX_ANISOTROPY, 16);

        // sampler1
        glGenSamplers(1, &renderer->sampler1_id);
        glSamplerParameteri(
          renderer->sampler1_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(
          renderer->sampler1_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindSampler(0, renderer->sampler0_id);
#endif // TESTING_GL_SAMPLER_OBJECTS

        // Send ECS event init
        gsk_ecs_event(ecs, ECS_INIT);

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
    gsk_Scene *p_active_scene = renderer->sceneL[renderer->activeScene];

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

    if (_gsk_device_check_fixed_update())
    {
        gsk_ecs_event(ecs, ECS_INIT); // call init at fixed (does not call on
                                      // entities that are already initialized)

        gsk_ecs_event(ecs, ECS_DESTROY);

        // fixed-update related events
        gsk_ecs_event(ecs, ECS_ON_COLLIDE);
        gsk_ecs_event(ecs, ECS_FIXED_UPDATE);
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
    renderer->currentPass              = GskRenderPass_GBuffer;
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
    gsk_Light directional_light = p_active_scene->lighting_data.lights[0];
    gsk_lighting_update(&p_active_scene->lighting_data);

    // update shadowmap position
    shadowmap_update(directional_light.position, renderer->shadowmapOptions);
    // TODO: Move matrix storage out of renderer
    glm_mat4_zero(renderer->lightSpaceMatrix);
    glm_mat4_copy(shadowmap_getMatrix(), renderer->lightSpaceMatrix);

    // bind the shadowmap textures & framebuffers
    shadowmap_bind();

    renderer->currentPass              = GskRenderPass_Shadowmap;
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

    vec4 clear_col = {0.0f, 0.0f, 0.0f, 1.0f};
    glm_vec3_copy(p_active_scene->fogOptions.fog_color, clear_col);
    glClearColor(clear_col[0], clear_col[1], clear_col[2], 1.0f);

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

#if TESTING_GLSAMPLER_OBJECTS
    glBindSampler(0, renderer->sampler0_id);
#endif // TESTING_GLSAMPLER_OBJECTS

#if LIGHTING_CULL_GLOBAL
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
#endif

    // Forward-draw Event
    renderer->currentPass = GskRenderPass_Lighting;
    gsk_ecs_event(ecs, ECS_RENDER);

#if LIGHTING_CULL_GLOBAL
    glDisable(GL_CULL_FACE);
#endif

    glDepthFunc(GL_LEQUAL);
    renderer->currentPass = GskRenderPass_Skybox;
    gsk_ecs_event(ecs, ECS_RENDER);
    glDepthFunc(GL_LESS);

    // Render skybox (NOTE: Look into whether we want to keep this in
    // the postprocessing buffer as it is now)
    if (scene->has_skybox)
    {
        glDepthFunc(GL_LEQUAL);
        gsk_skybox_draw(renderer->activeSkybox);
        glDepthFunc(GL_LESS);
    }

#if TESTING_GLSAMPLER_OBJECTS
    // reset texture unit sampler object
    glBindSampler(0, 0);
#endif // TESTING_GLSAMPLER_OBJECTS

    glPopDebugGroup();
    /*-------------------------------------------
        Pass #4 - Bloom Stage
    */

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 4, -1, "Pass: Bloom");
    u32 cnt_draw_id = postbuffer_get_id();
    pass_bloom_render(cnt_draw_id, &renderer->properties);

    glPopDebugGroup();

    /*-------------------------------------------
        Pass #5 - Final: Backbuffer draw
    */
    glPushDebugGroup(
      GL_DEBUG_SOURCE_APPLICATION, 5, -1, "Pass: Backbuffer Draw (Final)");

    postbuffer_draw(&renderer->properties, pass_bloom_get_texture_id());

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
    renderer->currentPass = GskRenderPass_Lighting;
    gsk_ecs_event(ecs, ECS_RENDER);

    vulkan_render_draw(renderer->vulkanDevice, renderer->window);
}
*/

void
gsk_renderer_tick(gsk_Renderer *renderer)
{
    gsk_Scene *scene = renderer->sceneL[renderer->activeScene];
    gsk_ECS *ecs     = scene->ecs;

    if (renderer->scene_queue_index != renderer->activeScene)
    {
        LOG_INFO("Switching scene from tick");
        gsk_renderer_active_scene(renderer, renderer->scene_queue_index);
    }

    if (GSK_DEVICE_API_OPENGL)
    {
        renderer_tick_OPENGL(renderer, scene, ecs);
    } else if (GSK_DEVICE_API_VULKAN)
    {
        // renderer_tick_VULKAN(renderer, ecs);
    }
}

void
gsk_renderer_resize(gsk_Renderer *p_self, int new_width, int new_height)
{
    // --
    // update window information

    p_self->windowWidth         = new_width;
    p_self->windowHeight        = new_height;
    p_self->window_aspect_ratio = (f32)new_width / (f32)new_height;

    // --
    // update pipeline

    // update OpenGL viewport
    if (GSK_DEVICE_API_OPENGL) { glViewport(0, 0, new_width, new_height); }

    postbuffer_resize((u32)new_width, (u32)new_height);
}
