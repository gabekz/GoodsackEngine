#include "renderer.h"

#include<stdio.h>

#include "gfx.h" // GLFW & glad headers
#include <util/debug.h>
#include <util/sysdefs.h>

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

#if 0
void renderer_tick(Renderer *renderer) {

    // create shadowmap
    shadowmap_create();

    // create postprocess buffer
    postbuffer_create();

    while(!glfwWindowShouldClose(renderer->window)) {

        camera_send_matrix(cameraIndex);
        
        renderer_update();

        // calculate deltaTime()
        if(deltaTimeTick) {
            renderer_fixedupdate();
        }
        //

        //PASS 1 Shadowmap
        shadowmap_bind();
        scene_draw(sceneIndex);

        for(int i = 0; i < meshCount; i++) {
            mesh_draw(meshL[i]);
        }

        //PASS 2 PostProcessing
        postbuffer_bind();
        scene_draw(sceneIndex);
        //

        //PASS 3 Backbuffer
        // -- TODO: set glFramebuffer
        glfwSwapBuffers(renderer->window);
        glfwPollEvents(); // + TODO: camera_input()
    }

    // Cleanup
}
#endif
