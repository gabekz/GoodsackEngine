#include "renderer.h"

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

    Renderer *ret = malloc(sizeof(Renderer));
    ret->window = window;
    ret->windowWidth = winWidth;
    ret->windowHeight = winHeight;

    return ret;
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
