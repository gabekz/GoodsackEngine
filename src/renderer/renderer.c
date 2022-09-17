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

    Scene *scene = renderer->sceneL[renderer->activeScene];

    // create shadowmap
    //shadowmap_create();

    // create postprocess buffer
    //postbuffer_create();

    while(!glfwWindowShouldClose(renderer->window)) {

    float  rotation     = 0.0f;
    float  rotationInc  = 0.5f;
    double timePrev     = -1.0f;

    camera_send_matrix(camera, 45.0f, 0.1f, 100.0f);

/*------------------------------------------- 
    |Pass #1 - Directional Shadowmap 
*/ 
    postbuffer_bind();
    scene_update(scene);
    scene_draw(scene);
    postbuffer_draw();
    glfwSwapBuffers(renderer->window);
    glfwPollEvents();
    camera_input(camera, renderer->window);
    }

    // Cleanup
}
