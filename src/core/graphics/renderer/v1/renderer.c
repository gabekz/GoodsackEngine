#include "renderer.h"
#include <core/graphics/renderer/pipeline/pass_compute.h>
#include <core/graphics/renderer/pipeline/pipeline.h>

#include <stdio.h>

#include <util/gfx.h>
#include <util/logger.h>
#include <util/sysdefs.h>

#include <core/graphics/lighting/lighting.h>
#include <core/graphics/lighting/skybox.h>
#include <core/graphics/ui/billboard.h>
#include <core/graphics/ui/gui_element.h>
#include <core/graphics/ui/gui_text.h>
#include <entity/v1/ecs.h>

#include <core/device/device_context.h>

#include <core/device/device.h>
#include <core/drivers/vulkan/vulkan_device.h>

// Skybox test
#include <core/graphics/texture/texture.h>

#include <tools/debug/debug_context.h>
#include <tools/debug/debug_draw_line.h>

#define TESTING_DRAW_UI   1
#define TESTING_DRAW_LINE 0

Renderer *
renderer_init()
{
    int winWidth  = DEFAULT_WINDOW_WIDTH;
    int winHeight = DEFAULT_WINDOW_HEIGHT;

    Renderer *ret = malloc(sizeof(Renderer));
    GLFWwindow *window =
      /*context*/ createWindow(winWidth, winHeight, &ret->vulkanDevice);

    ret->window       = window;
    ret->windowWidth  = winWidth;
    ret->windowHeight = winHeight;

    // Set Render Resolution
    ret->renderWidth =
      (RENDER_RESOLUTION_OVERRIDE) ? PSX_WIDTH : DEFAULT_WINDOW_WIDTH;
    ret->renderHeight =
      (RENDER_RESOLUTION_OVERRIDE) ? PSX_HEIGHT : DEFAULT_WINDOW_HEIGHT;

    // Create the initial scene
    Scene *scene      = malloc(sizeof(Scene));
    scene->id         = 0;
    scene->ecs        = ecs_init(ret);
    scene->has_skybox = FALSE;

    Scene **sceneList = malloc(sizeof(Scene *));
    *(sceneList)      = scene;

    ret->sceneL      = sceneList;
    ret->sceneC      = 1;
    ret->activeScene = 0;

    ret->properties = (RendererProps) {.tonemapper      = 0,
                                       .exposure        = 2.5f,
                                       .maxWhite        = 1.0f,
                                       .gamma           = 2.2f,
                                       .gammaEnable     = TRUE,
                                       .msaaEnable      = TRUE,
                                       .msaaSamples     = 4,
                                       .vignetteAmount  = 0.5f,
                                       .vignetteFalloff = 0.5f};

    ret->shadowmapOptions = (ShadowmapOptions) {
      .nearPlane = 0.02f,
      .farPlane  = 20.0f,
      .camSize   = 2.0f,

      .normalBiasMin = 0.0025f,
      .normalBiasMax = 0.0005f,
      .pcfSamples    = 6,
    };

    ret->ssaoOptions = (SsaoOptions) {
      .strength   = 2.1f,
      .bias       = 0.0003f,
      .radius     = 0.15f,
      .kernelSize = 16,
    };

    // Billboard test
    vec2 bbsize = {0.01f, 0.01f};
    ret->billboard =
      billboard_create("../res/textures/gizmo/light.png", bbsize);

    // GUI test
    Texture *guiTexture =
      texture_create("../res/textures/gizmo/crosshair2.png",
                     NULL,
                     (TextureOptions) {1, GL_RGBA, false, true});
    ret->uiImage = gui_element_create(
      (vec2) {1920 / 2, 1080 / 2}, (vec2) {10, 10}, guiTexture, NULL);

    // Test GUI Text
    ret->uiText = gui_text_create("Goodsack");

    return ret;
}

ECS *
renderer_active_scene(Renderer *self, ui16 sceneIndex)
{
    LOG_INFO("Loading scene: id %d", sceneIndex);
    ui32 sceneCount = self->sceneC;
    if (sceneCount < sceneIndex + 1) {
        LOG_INFO(
          "Scene %d does not exist. Creating Scene %d", sceneIndex, sceneIndex);
        ui32 newCount = sceneIndex - sceneCount + (sceneCount + 1);

        // Create a new, empty scene
        Scene *newScene      = malloc(sizeof(Scene));
        newScene->id         = newCount;
        newScene->ecs        = ecs_init(self);
        newScene->has_skybox = FALSE;

        // Update the scene list
        Scene **p                  = self->sceneL;
        self->sceneL               = realloc(p, newCount * sizeof(Scene *));
        self->sceneL[newCount - 1] = newScene;
        self->sceneC               = newCount;
    }

    self->activeScene = sceneIndex;

    return self->sceneL[sceneIndex]->ecs;

    // TODO: add checks here and cleanup from previous scene for switching.
}

void
renderer_start(Renderer *renderer)
{
    // Scene initialization
    Scene *scene = renderer->sceneL[renderer->activeScene];
    ECS *ecs     = scene->ecs;

    if (DEVICE_API_OPENGL) {

// Create the default skybox
#if 0
        Texture *skyboxCubemap =
          texture_create_cubemap(6,
                                 "../res/textures/skybox/right.jpg",
                                 "../res/textures/skybox/left.jpg",
                                 "../res/textures/skybox/top.jpg",
                                 "../res/textures/skybox/bottom.jpg",
                                 "../res/textures/skybox/front.jpg",
                                 "../res/textures/skybox/back.jpg");

        renderer->defaultSkybox = skybox_create(skyboxCubemap);
#else
        renderer->defaultSkybox = skybox_hdr_create(
          texture_create_hdr("../res/textures/hdr/thatch_chapel_4k.hdr"));
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

        // renderer->skybox = skybox_create(skyboxCubemap);

        // Send ECS event init
        ecs_event(ecs, ECS_INIT);

        // glEnable(GL_FRAMEBUFFER_SRGB);
        clearGLState();

        renderer->debugContext = debug_context_init();

        // TESTING Compute Shaders
        // computebuffer_init();

        // render image to quad

    } else if (DEVICE_API_VULKAN) {
        ecs_event(ecs, ECS_INIT);
        // LOG_DEBUG("Renderer Start-Phase is not implemented in Vulkan");
    }
}

/* Render Functions for the pipeline */

static void
renderer_tick_OPENGL(Renderer *renderer, Scene *scene, ECS *ecs)
{
    // Settings
    glfwSwapInterval(device_getGraphicsSettings().swapInterval);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /*-------------------------------------------
        Scene Logic/Data update
    */

    device_setInput(device_getInput()); // TODO: Weird hack to reset for axis
    device_updateCursorState(renderer->window);
    glfwPollEvents();

    ecs_event(ecs, ECS_UPDATE);
    ecs_event(ecs, ECS_LATE_UPDATE);

    /*-------------------------------------------
        Pass #0 - Depth Prepass
    */
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Pass: Depth Prepass");

    prepass_bind();
    renderer->currentPass      = DEPTH_PREPASS;
    renderer->explicitMaterial = prepass_getMaterial();
    ecs_event(ecs, ECS_RENDER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glPopDebugGroup();
    /*-------------------------------------------
        Pass #1 - Directional Shadowmap
    */
    glPushDebugGroup(
      GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Pass: Shadowmap Depth");

    // update lighting information
    lighting_update(
      renderer->light, renderer->light->position, renderer->light->color);
    // update shadowmap position
    shadowmap_update(renderer->light->position, renderer->shadowmapOptions);
    // TODO: Move matrix storage out of renderer
    glm_mat4_zero(renderer->lightSpaceMatrix);
    glm_mat4_copy(shadowmap_getMatrix(), renderer->lightSpaceMatrix);

    // bind the shadowmap textures & framebuffers
    shadowmap_bind();

    renderer->currentPass      = SHADOW;
    renderer->explicitMaterial = shadowmap_getMaterial();
    // TODO: Clean this up...
    ecs_event(ecs, ECS_RENDER);

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
    if (scene->has_skybox) {
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
    ecs_event(ecs, ECS_RENDER);

    // Render skybox (NOTE: Look into whether we want to keep this in
    // the postprocessing buffer as it is now)
    if (scene->has_skybox) {
        glDepthFunc(GL_LEQUAL);
        skybox_draw(renderer->activeSkybox);
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
    // billboard_draw(renderer->billboard, pos);

    gui_element_draw(renderer->uiImage);
    gui_text_draw(renderer->uiText);
#endif

#if TESTING_DRAW_LINE
    vec3 start = {0, 0, 0};
    vec3 end   = {-20, 20, 0};
    debug_draw_line(renderer->debugContext, start, end);
#endif

    // computebuffer_draw();
}

/*
void renderer_tick_VULKAN(Renderer *renderer, ECS *ecs) {

// Update Analytics Data

    glfwPollEvents();

    ecs_event(ecs, ECS_UPDATE);
    renderer->currentPass = REGULAR;
    ecs_event(ecs, ECS_RENDER);

    vulkan_render_draw(renderer->vulkanDevice, renderer->window);
}
*/

void
renderer_tick(Renderer *renderer)
{
    Scene *scene = renderer->sceneL[renderer->activeScene];
    ECS *ecs     = scene->ecs;

    if (DEVICE_API_OPENGL) {
        renderer_tick_OPENGL(renderer, scene, ecs);
    } else if (DEVICE_API_VULKAN) {
        // renderer_tick_VULKAN(renderer, ecs);
    }
}
