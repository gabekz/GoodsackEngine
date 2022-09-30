#include "../renderer.h"

#include<stdio.h>

#include <util/debug.h>
#include <util/gfx.h>
#include <util/sysdefs.h>

#include "postbuffer.h"
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
   glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, DEBUG);

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
        ui32 newCount = sceneCount + sceneIndex;

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

// Create the depthmap 
    ui32 depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // Texture
    ui32 depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mat4 lightProjection    = GLM_MAT4_ZERO_INIT;
    mat4 lightView          = GLM_MAT4_ZERO_INIT;
    mat4 lightSpaceMatrix   = GLM_MAT4_ZERO_INIT;
    float nearPlane = 0.5f, farPlane = 7.5f;
    float camSize = 10.0f;
    glm_ortho(-camSize, camSize, -camSize, camSize,
        nearPlane, farPlane, lightProjection);
    glm_lookat(
            (vec3){1.0f, 1.0f, 1.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 1.0f, 0.0f}, lightView);
    glm_mat4_mul(lightProjection, lightView, lightSpaceMatrix);

    ShaderProgram *shaderDepthMap =
        shader_create_program("../res/shaders/depth-map.shader");

    Material *materialDepthMap = material_create(shaderDepthMap, NULL, 0);

    // TODO: Again, keeping the hacky shit up here.
    renderer->shaderDepthMap = shaderDepthMap;
    renderer->materialDepthMap = materialDepthMap;
    renderer->depthMapFBO = depthMapFBO;
    renderer->depthMapTexture = depthMap;
    glm_mat4_copy(lightSpaceMatrix, renderer->lightSpaceMatrix);

// Send ECS event init
    ecs_event(ecs, ECS_INIT);

// Create the post buffer
    postbuffer_init(renderer->windowWidth, renderer->windowHeight);

// Clear GL state
    clearGLState();

// Enable Gamma correction
    glEnable(GL_FRAMEBUFFER_SRGB);

}

/* Render Functions for the pipeline */

void renderer_tick(Renderer *renderer) {
    Scene *scene = renderer->sceneL[renderer->activeScene];
    ECS *ecs = scene->ecs;

    // TODO: Keeping some pointers up here so we know where the hacky shit is.
    ShaderProgram *shaderDepthMap = renderer->shaderDepthMap;
    Material *materialDepthMap = renderer->materialDepthMap;
    ui32 depthMapFBO = renderer->depthMapFBO;
    ui32 depthMapTexture = renderer->depthMapTexture;

/*------------------------------------------- 
    Scene Logic/Data update
*/ 
    glfwPollEvents();
    ecs_event(ecs, ECS_UPDATE);

/*------------------------------------------- 
    Pass #1 - Directional Shadowmap 
*/ 
    shader_use(shaderDepthMap);
    glUniformMatrix4fv(
        glGetUniformLocation(shaderDepthMap->id, "u_LightSpaceMatrix"),
        1, GL_FALSE, (float *)renderer->lightSpaceMatrix);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderer->currentPass = SHADOW;
    renderer->explicitMaterial = materialDepthMap;
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

    // Bind the shadowmap to texture slot 6
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);

    renderer->currentPass = REGULAR;
    ecs_event(ecs, ECS_RENDER);

/*------------------------------------------- 
    Pass #3 - Final: Backbuffer draw
*/ 
    //glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the backbuffer
    postbuffer_draw(renderer->windowWidth, renderer->windowHeight);
}
