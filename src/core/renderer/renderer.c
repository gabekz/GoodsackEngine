#include "../renderer.h"
#include "pipeline.h"

#include<stdio.h>

#include <util/debug.h>
#include <util/gfx.h>
#include <util/sysdefs.h>

#include <core/ecs.h>

static void _resize_callback
(GLFWwindow* window, int widthRe, int heightRe) {
    printf("window resize: %d and %d\n", widthRe, heightRe);
    glViewport(0, 0, widthRe, heightRe);
}

static void _key_callback 
(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }
}

Renderer* renderer_init() {
   glfwSetErrorCallback(_error_callback);
   if(!glfwInit()) { // Initialization failed
        printf("Failed to initialize glfw");
   }

   // Minimum OpenGL version required
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   // debug ALL OpenGL Errors
   //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, DEBUG);

   int winWidth    = DEFAULT_WINDOW_WIDTH;
   int winHeight   = DEFAULT_WINDOW_HEIGHT;

   GLFWwindow* window =
      glfwCreateWindow(winWidth, winHeight, "Title", NULL, NULL);

   if(!window) printf("Failed to create window");

   // Set the context and load GL [Note: different for Vk]
   glfwMakeContextCurrent(window);
   gladLoadGL(glfwGetProcAddress);

   glfwGetFramebufferSize(window, &winWidth, &winHeight);
   glfwSetFramebufferSizeCallback(window, _resize_callback);
   glfwSetKeyCallback(window, _key_callback);

    // Initialize GL debug callback
    glDebugInit();

   // Get current OpenGL version
   printf("%s\n", glGetString(GL_VERSION));

   // Refresh rate 
   glfwSwapInterval(1);

    Renderer *ret = malloc(sizeof(Renderer));
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
    ui32 sceneCount = self->sceneC;
    if(sceneCount < sceneIndex + 1) {
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

    shadowmap_init();
    // TODO: clean this up. Should be stored in UBO for directional-lights
    glm_mat4_zero(renderer->lightSpaceMatrix);
    glm_mat4_copy(shadowmap_getMatrix(), renderer->lightSpaceMatrix);

    postbuffer_init(renderer->windowWidth, renderer->windowHeight);


// Send ECS event init
    ecs_event(ecs, ECS_INIT);

    glEnable(GL_FRAMEBUFFER_SRGB);
    clearGLState();
}

/* Render Functions for the pipeline */

void renderer_tick(Renderer *renderer) {
    Scene *scene = renderer->sceneL[renderer->activeScene];
    ECS *ecs = scene->ecs;

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // binding the shadowmap to texture slot 6 (TODO:) for meshes
    shadowmap_bind_texture();

    renderer->currentPass = REGULAR;
    ecs_event(ecs, ECS_RENDER);

/*------------------------------------------- 
    Pass #3 - Final: Backbuffer draw
*/ 
    //glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the backbuffer
    postbuffer_draw(renderer->windowWidth, renderer->windowHeight);
}
