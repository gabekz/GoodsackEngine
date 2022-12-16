#include "renderer.h"
#include "pipeline/pipeline.h"

#include<stdio.h>

#include <util/debug.h>
#include <util/logger.h>
#include <util/gfx.h>
#include <util/sysdefs.h>

#include <ecs/ecs.h>
#include <core/lighting/lighting.h>

#include <core/context/context.h>

// API
#include <core/api/device_api.h>
#include <core/api/vulkan/vulkan_device.h>

// Skybox test
#include <core/texture/texture.h>

Renderer* renderer_init() {
   int winWidth    = DEFAULT_WINDOW_WIDTH;
   int winHeight   = DEFAULT_WINDOW_HEIGHT;


    Renderer *ret = malloc(sizeof(Renderer));
    GLFWwindow *window = createWindow(winWidth, winHeight, 
            &ret->vulkanDevice);

    ret->window       = window;
    ret->windowWidth  = winWidth;
    ret->windowHeight = winHeight;

// Create the initial scene
    Scene *scene = malloc(sizeof(Scene));

    scene->id = 0;
    scene->ecs = ecs_init(ret);

    Scene **sceneList = malloc(sizeof(Scene*));
    *(sceneList) = scene;

    ret->sceneL      = sceneList;
    ret->sceneC      = 1;
    ret->activeScene = 0;

    return ret;
}

ECS *renderer_active_scene(Renderer* self, ui16 sceneIndex) {
    LOG_INFO("Loading scene: id %d", sceneIndex);
    ui32 sceneCount = self->sceneC;
    if(sceneCount < sceneIndex + 1) {
        LOG_INFO("Scene %d does not exist. Creating Scene %d", sceneIndex, sceneIndex);
        ui32 newCount = sceneIndex - sceneCount + (sceneCount + 1);

        // Create a new, empty scene
        Scene *newScene = malloc(sizeof(Scene));
        newScene->id = newCount;
        newScene->ecs = ecs_init(self);

        // Update the scene list
        Scene **p = self->sceneL;
        self->sceneL = realloc(p, newCount * sizeof(Scene*));
        self->sceneL[newCount-1] = newScene;
        self->sceneC = newCount;
    }

    self->activeScene = sceneIndex;

    return self->sceneL[sceneIndex]->ecs;

    // TODO: add checks here and cleanup from previous scene for switching.
}

void renderer_start(Renderer *renderer) {
// Scene initialization
    Scene *scene = renderer->sceneL[renderer->activeScene];
    ECS *ecs = scene->ecs;

    if(DEVICE_API_OPENGL) {
        shadowmap_init();
        // TODO: clean this up. Should be stored in UBO for directional-lights
        glm_mat4_zero(renderer->lightSpaceMatrix);
        glm_mat4_copy(shadowmap_getMatrix(), renderer->lightSpaceMatrix);

        postbuffer_init(renderer->windowWidth, renderer->windowHeight);

#if 1
        Texture *skyboxCubemap = texture_create_cubemap(6, 
            "../res/textures/skybox/right.jpg",
            "../res/textures/skybox/left.jpg",
            "../res/textures/skybox/top.jpg",
            "../res/textures/skybox/bottom.jpg",
            "../res/textures/skybox/front.jpg",
            "../res/textures/skybox/back.jpg"
        );
#else
        Texture *skyboxCubemap = texture_create_hdr("../res/textures/hdr/ice_lake.hdr");
#endif

        renderer->skybox = skybox_create(skyboxCubemap);

// Send ECS event init
        ecs_event(ecs, ECS_INIT);

        //glEnable(GL_FRAMEBUFFER_SRGB);
        clearGLState();
    }
    else if (DEVICE_API_VULKAN) {
        LOG_DEBUG("Renderer Start-Phase is not implemented in Vulkan");
    }
}

/* Render Functions for the pipeline */

static void renderer_tick_OPENGL(Renderer *renderer, Scene *scene, ECS *ecs) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

/*------------------------------------------- 
    Scene Logic/Data update
*/ 
    glfwPollEvents();
    ecs_event(ecs, ECS_UPDATE);

/*------------------------------------------- 
    Pass #1 - Directional Shadowmap 
*/ 
    shadowmap_bind();
    renderer->currentPass = SHADOW;
    renderer->explicitMaterial = shadowmap_getMaterial();
    // TODO: Clean this up...
    ecs_event(ecs, ECS_RENDER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);

/*------------------------------------------- 
    Pass #2 - Post Processing Pass 
*/ 
    postbuffer_bind();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // binding the shadowmap to texture slot 6 (TODO:) for meshes
    shadowmap_bind_texture();

    renderer->currentPass = REGULAR;
    ecs_event(ecs, ECS_RENDER);

    // Render skybox (NOTE: Look into whether we want to keep this in
    // the postprocessing buffer as it is now)
    glDepthFunc(GL_LEQUAL);
    skybox_draw(renderer->skybox);
    glDepthFunc(GL_LESS);

/*------------------------------------------- 
    Pass #3 - Final: Backbuffer draw
*/ 
    //glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the backbuffer
    postbuffer_draw(renderer->windowWidth, renderer->windowHeight);
}

void renderer_tick_VULKAN(Renderer *renderer, ECS *ecs) {
    glfwPollEvents();
    //ecs_event(ecs, ECS_UPDATE);

    //renderer->currentPass = REGULAR;
    //ecs_event(ecs, ECS_RENDER);

    vulkan_drawFrame(renderer->vulkanDevice);

}

void renderer_tick(Renderer *renderer) {
    Scene *scene = renderer->sceneL[renderer->activeScene];
    ECS *ecs = scene->ecs;

    if(DEVICE_API_OPENGL) {
        renderer_tick_OPENGL(renderer, scene, ecs);
    }
    else if(DEVICE_API_VULKAN) {
        renderer_tick_VULKAN(renderer, ecs);
    }
}
