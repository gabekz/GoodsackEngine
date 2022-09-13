/* -- main.c

   Main entry. Handling initialization.

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <cglm/cglm.h>
#include <cglm/struct.h>

#include <util/sysdefs.h>

#include "gfx.h" // GLFW & glad headers
#include "shader.h"

#include "glbuffer/glbuffer.h"
#include "mesh.h"
#include "texture.h"

#include "primitives.h"
#include "camera.h"

#include "loaders/loader_obj.h"

static int winWidth  = DEFAULT_WINDOW_WIDTH;
static int winHeight = DEFAULT_WINDOW_HEIGHT;

/* ~~~ CALLBACKS ~~~ */

static void _error_callback
(int error, const char* description) {
   fprintf(stderr, "Error %s\n", description);
}

static void _key_callback 
(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }
}

static void _resize_callback(GLFWwindow* window, int widthRe, int heightRe) {
    printf("window resize: %d and %d\n", widthRe, heightRe);
    glViewport(0, 0, widthRe, heightRe);
}

/* ~~~ ERROR HANDLING ~~~ */

static void GLClearError() {
   while(glGetError() != GL_NO_ERROR);
}

static void GLCheckError() {
   GLenum error = glGetError();
   while(error) {
      printf("\n|OpenGL Error| (%s)\n", error);
   }
}

/* ~~ Clear ~~ */
void clearGL() {
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);            // unbind VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    // unbind IBO
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

/* ~~~ MAIN ~~~ */

int main(void) {
   glfwSetErrorCallback(_error_callback);
   if(!glfwInit()) { // Initialization failed
        printf("Failed to initialize glfw");         
   }

   // Minimum OpenGL version required
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   int winWidth    = DEFAULT_WINDOW_WIDTH;
   int winHeight   = DEFAULT_WINDOW_HEIGHT;

   GLFWwindow* window =
      glfwCreateWindow(winWidth, winHeight, "My Title", NULL, NULL);

   if(!window) printf("Failed to create window");

   // Set the context and load GL [Note: different for Vk]
   glfwMakeContextCurrent(window);
   gladLoadGL(glfwGetProcAddress);

   glfwGetFramebufferSize(window, &winWidth, &winHeight);
   glfwSetFramebufferSizeCallback(window, _resize_callback);

   // Set callbacks
   glfwSetKeyCallback(window, _key_callback);

   // Refresh rate 
   glfwSwapInterval(1);
   
   // Get current OpenGL version
   printf("%s\n", glGetString(GL_VERSION));

#ifdef CIMGUI
   // IMGUI initialization
   imgui_init(window);
#endif

// Create the Camera, containing starting-position and up-axis coords.
    // TODO: Update camera when window is resized
    Camera* camera = camera_create(winWidth, winHeight,
      (vec3){0.0f, 0.0f, 2.0f}, (vec3){0.0f, 1.0f, 0.0f});

// Lighting information
    float* lightPos     = (vec3){0.0f, 0.5f, 0.4f};
    float* lightColor   = (vec4){1.0f, 1.0f, 1.0f, 1.0f};

// UBO Lighting
    ui32 uboLight;
    ui32 uboLightSize = sizeof(vec3) + 4 + sizeof(vec4);
    glGenBuffers(1, &uboLight);
    glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
    glBufferData(GL_UNIFORM_BUFFER, uboLightSize, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, uboLight, 0, uboLightSize);
    // Send lighting data to UBO
    glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
    glBufferSubData(GL_UNIFORM_BUFFER,
        0, sizeof(vec3) + 4,
        lightPos);
    glBufferSubData(GL_UNIFORM_BUFFER,
        sizeof(vec3) + 4, sizeof(vec4),
        lightColor);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

// Framebuffer [Post-Processing]
    // Shader
    ui32 fbShader = CreateShader("../res/shaders/framebuffer.shader");
    glUseProgram(fbShader);
    glUniform1i(glGetUniformLocation(fbShader, "u_ScreenTexture"), 0);

    // Create Rectangle
    VAO *vaoRect = vao_create();
    vao_bind(vaoRect);
    float *rectPositions = prim_vert_rect();

    VBO *vboRect = vbo_create(rectPositions, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vboRect);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vaoRect, vboRect);
    free(rectPositions);

    // Create Texture
#if 1
    ui32 frameBufferTexture;
    glGenTextures(1, &frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, winWidth, winHeight,
        0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
    Texture *frameBufferTexture = texture_create(NULL, 0);
#endif

    // Create Framebuffer object
    ui32 FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, frameBufferTexture, 0);

    // Create Renderbuffer object [Depth]
    ui32 RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
        winWidth, winHeight);
    //glBindRenderbuffer(GL_RENDERBUFFER, 0); -- This doesn't have to be done?
    // Attach Renderbuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, RBO);

    // Error checking
    GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(fbStatus != GL_FRAMEBUFFER_COMPLETE) {
        printf("\nFramebuffer ERROR: %u\n", fbStatus);
    }

// Create an object with the OBJ Loader
    VAO *vaoSuzanne = load_obj("../res/models/suzanne.obj");
    // Create texture
    Texture *tex = texture_create(
      "../res/textures/bricks.png", 0);
    // Shader
    unsigned int shaderSuzanne =
        CreateShader("../res/shaders/lit-diffuse.shader");
    glUseProgram(shaderSuzanne);
    // Send texture location to shader
    glUniform1i(glGetUniformLocation(shaderSuzanne, "u_Texture"), 0);

// Create the point-light object
   VAO *vaoLight = vao_create();
   vao_bind(vaoLight);
    // Cube for point-light
    float *lightPositions = prim_vert_cube(0.03f);
    VBO *vboLight = vbo_create(lightPositions, (3 * 8) * sizeof(float));
    free(lightPositions);
    vbo_push(vboLight, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vaoLight, vboLight);
    // Shader
    unsigned int shaderLightObj = CreateShader("../res/shaders/white.shader");

// Clearing GL State
    clearGL();
    // Free VBO's [not needed anymore, the data exists in the VAO's]
    //free(vbLight);

// Rotation parametes for the render loop
    float  rotation     = 0.0f;
    float  rotationInc  = 0.5f;
    double timePrev     = -1.0f;

// Render Loop
    while(!glfwWindowShouldClose(window)) {

    // Bind the Framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // Start by clearing our buffer bits and setting a BG color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.15f, 1.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CW);

    // rotation logic for model 
        if((glfwGetTime() - timePrev) >= (1.0f / 60.0f)) {
            rotation += rotationInc;
        }

    // Update the view and projection based on the camera data
        camera_send_matrix(camera, 45.0f, 0.1f, 100.0f);

    // Drawing our first object
        glUseProgram(shaderSuzanne);
        // Calculating model
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        glm_rotate(model, glm_rad(rotation), (vec3){0.0f, 1.0f, 0.0f});
        int modelLoc = glGetUniformLocation(shaderSuzanne, "u_Model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)model);
        // Bind and draw
        vao_bind(vaoSuzanne);
        texture_bind(tex);
        glDrawArrays(DRAWING_MODE, 0, 23232);

    // drawing our second object
        glUseProgram(shaderLightObj);
        // Calculating model
        mat4 model2 = GLM_MAT4_IDENTITY_INIT;
        glm_translate(model2, lightPos);
        glUniformMatrix4fv(glGetUniformLocation(shaderLightObj, "u_Model"),
          1, GL_FALSE, (float *)model2);
        // Bind and draw
        vao_bind(vaoLight);
        glDisable(GL_CULL_FACE);
        glDrawArrays(DRAWING_MODE, 0, 24);
 
    // Second pass [Post-Processing buffer]
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind the backbuffer
        vao_bind(vaoRect);
        glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glUseProgram(fbShader);
        glDrawArrays(GL_TRIANGLES, 0, 6);

// Swap backbuffer + poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();
        camera_input(camera, window);
    }

// Clean-up 
    glDeleteProgram(shaderSuzanne);
    glDeleteProgram(shaderLightObj);
    glDeleteProgram(fbShader);
    // TODO: destory buffers HERE
    glfwDestroyWindow(window);
    glfwTerminate();

} /* end of main.c */
