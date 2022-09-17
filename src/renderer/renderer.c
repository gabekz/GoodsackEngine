#include "renderer.h"

#include<stdio.h>

#include "gfx.h" // GLFW & glad headers
#include <util/debug.h>
#include <util/sysdefs.h>

#include "../camera.h"

#include "postbuffer.h"

static void _resize_callback(GLFWwindow* window, int widthRe, int heightRe) {
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

// Create the initial scene
    Scene *scene = malloc(sizeof(Scene));

    scene->meshL = calloc(1, sizeof(Mesh*));
    scene->meshC = 0;

    scene->lightL = calloc(1, sizeof(Light*));
    scene->lightC = 0;

    Scene **sceneList = malloc(sizeof(Scene*));
    *(sceneList) = scene;


    Renderer *ret = malloc(sizeof(Renderer));
    ret->window       = window;
    ret->windowWidth  = winWidth;
    ret->windowHeight = winHeight;

    ret->sceneL      = sceneList;
    ret->sceneC      = 1;
    ret->activeScene = 0;

    return ret;
}

void renderer_active_scene(Renderer* self, ui16 sceneIndex) {
    ui32 sceneCount = self->sceneC;
    if(sceneCount < sceneIndex + 1) {
        void *p = (void *)self->sceneL;
        self->sceneL = realloc(p, sceneCount + sceneIndex * sizeof(Scene*));
    }

    self->activeScene = sceneIndex;

    // TODO: add checks here and cleanup from previous scene for switching.
}

void renderer_add_mesh(Renderer *self, Mesh* mesh) {
    Scene* scene = self->sceneL[self->activeScene];
    int count = scene->meshC + 1;

    void *p = (void *)scene->meshL;
    scene->meshL = realloc(p, count * sizeof(Mesh*));
    scene->meshC = count;

    scene->meshL[count-1] = mesh;
}

void renderer_tick(Renderer *renderer, Camera *camera) {

// Scene initialization
    Scene *scene = renderer->sceneL[renderer->activeScene];

// Create the depthmap 
    ui32 depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // Texture
    const ui32 SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    ui32 depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mat4 lightProjection    = GLM_MAT4_ZERO_INIT;
    mat4 lightView          = GLM_MAT4_ZERO_INIT;
    mat4 lightSpaceMatrix   = GLM_MAT4_ZERO_INIT;
    float nearPlane = 1.0f, farPlane = 7.5f;
    glm_ortho(-10.0f, 10.0f, -10.0f, 10.0f,
        nearPlane, farPlane, lightProjection);
    glm_lookat(
            (vec3){-2.0f, 4.0f, -1.0f},
            (vec3){0.0f, 0.0f, 0.0f},
            (vec3){0.0f, 1.0f, 0.0f}, lightView);
    glm_mat4_mul(lightProjection, lightView, lightSpaceMatrix);

    ShaderProgram *shaderDepthMap =
        shader_create_program("../res/shaders/depth-map.shader");

    Material *materialDepthMap = material_create(shaderDepthMap, 0);

// Create the post buffer
    postbuffer_init(renderer->windowWidth, renderer->windowHeight);

// Clear GL state
    clearGLState();

/*------------------------------------------- 
|   Render Loop
*/
    while(!glfwWindowShouldClose(renderer->window)) {
/*------------------------------------------- 
    Scene Logic/Data update
*/ 
        glfwPollEvents();
        camera_input(camera, renderer->window);
        camera_send_matrix(camera, 45.0f, 0.1f, 100.0f);
        scene_update(scene);
/*------------------------------------------- 
    Pass #1 - Directional Shadowmap 
*/ 
        shader_use(shaderDepthMap);
        glUniformMatrix4fv(
            glGetUniformLocation(shaderDepthMap->id, "u_LightSpaceMatrix"),
            1, GL_FALSE, (float *)lightSpaceMatrix);

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        //scene_draw_explicit(renderer, materialDepthMap);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);

/*------------------------------------------- 
    Pass #2 - Post Processing Pass 
*/ 
        postbuffer_bind();
        scene_draw(scene);
/*------------------------------------------- 
    Pass #3 - Final: Backbuffer draw
*/ 
        postbuffer_draw();
        glfwSwapBuffers(renderer->window);
    }

    // Cleanup
}
