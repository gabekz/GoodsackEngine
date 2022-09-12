/* -- main.c

   Main entry. Handling initialization.

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <cglm/cglm.h>   /* for inline */
#include <cglm/struct.h>   /* for inline */

#include <util/sysdefs.h>

#include "gfx.h" // GLFW & glad headers
#include "shader.h"

#include "glbuffer/glbuffer.h"
#include "mesh.h"
#include "texture.h"

#include "primitives.h"
#include "camera.h"

#include "loaders/loader_obj.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define DRAWING_MODE GL_TRIANGLES

static int winWidth  = WINDOW_WIDTH;
static int winHeight = WINDOW_HEIGHT;
static int vertexCount = 4;

/* ~~~ CALLBACKS ~~~ */

static void _error_callback
(int error, const char* description) {
   fprintf(stderr, "Error %s\n", description);
}

static void _key_callback 
(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
   {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }
   if(key == GLFW_KEY_V && action == GLFW_PRESS)
   {
       vertexCount++;
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

   int winWidth    = WINDOW_WIDTH;
   int winHeight   = WINDOW_HEIGHT;

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

// Create the point-light object
   VAO *vaoLight = vao_create();
   vao_bind(vaoLight);
    // Cube for point-light
    float *lightPositions = prim_vert_cube(0.03f);
    VBO *vbLight = vbo_create(lightPositions, (3 * 8) * sizeof(float));
    free(lightPositions);

   unsigned int lightIndices[] = {
       0, 1, 2,
       2, 3, 0,
       0, 4, 7,
       7, 0, 3,
       3, 7, 6,
       6, 3, 2,
       2, 6, 5,
       5, 2, 1,
       1, 5, 4,
       1, 0, 4,
       4, 5, 6,
       6, 4, 7
   };
    IBO *ibLight = ibo_create(lightIndices , (3 * 12) * sizeof(unsigned int));

    vbo_push(vbLight, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vaoLight, vbLight);

// Loaded VAO
   VAO* vaoLoaded = load_obj("../res/models/suzanne.obj");

    // Set up for shaders
    unsigned int shader = CreateShader("../res/shaders/lit-diffuse.shader");
    glUseProgram(shader); // Activate the shader

// Setup for the second shader (test)
    unsigned int shader2 = CreateShader("../res/shaders/white.shader");

#ifdef MESH_ABSTRACTION_TEST

    unsigned int meshShader =
      CreateShader("../res/shaders/lit-diffuse.shader");

    Mesh *mesh = mesh_create("../res/models/suzanne.obj", meshShader);

    Mesh *meshSingle = mesh_create(
        "../res/models/suzanne.obj",            // model data
        "../res/shaders/lit-diffuse.shader",    // shader path 
        "../res/textures/bricks.png"            // texture [diffuse]
        );
#endif

    // Create texture
    Texture *tex = texture_create(
      (unsigned char*)"../res/textures/bricks.png");
    texture_bind(tex);
    // send it to the shader
    int location2 = glGetUniformLocation(shader, "u_Texture");
    glUniform1i(location2, 0); // 0 is the first (only) texture?

// Lights UBO
    ui32 uboLight;
    ui32 uboLightSize = sizeof(vec3) + 4 + sizeof(vec4);
    glGenBuffers(1, &uboLight);
    glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
    glBufferData(GL_UNIFORM_BUFFER, uboLightSize, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, uboLight, 0, uboLightSize);

// Lighting information
    float* lightPos     = (vec3){0.0f, 0.5f, 0.4f};
    float* lightColor   = (vec4){1.0f, 1.0f, 1.0f, 1.0f};

    // Send lighting data to UBO
    glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
    glBufferSubData(GL_UNIFORM_BUFFER,
        0, sizeof(vec3) + 4,
        lightPos);
    glBufferSubData(GL_UNIFORM_BUFFER,
        sizeof(vec3) + 4, sizeof(vec4),
        lightColor);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

// Framebuffer 
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
    ui32 frameBufferTexture;
    glGenTextures(1, &frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, winWidth, winHeight,
        0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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

    // Shader for the first model
        texture_bind(tex);
        glUseProgram(shader);

    // Rotate the model, send information to shader
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        glm_rotate(model, glm_rad(rotation), (vec3){0.0f, 1.0f, 0.0f});
        int modelLoc = glGetUniformLocation(shader, "u_Model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)model);

/* Testing the OBJ Loader. The object here can use the same shader
    and matrix information, both for the pyramid and loaded model. */
        vao_bind(vaoLoaded);
        glDrawArrays(DRAWING_MODE, 0, 23232);

    // drawing our second object
        glUseProgram(shader2);

        mat4 model2 = GLM_MAT4_IDENTITY_INIT;
        glm_translate(model2, lightPos);
        glUniformMatrix4fv(glGetUniformLocation(shader2, "u_Model"),
          1, GL_FALSE, (float *)model2);

        vao_bind(vaoLight);
        glDrawElements(DRAWING_MODE, 36, GL_UNSIGNED_INT, NULL); 
 
    // Compute camera-input
        camera_input(camera, window);

// Second pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(fbShader);
    vao_bind(vaoRect);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Swap backbuffer and poll for GLFW events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

// Clean-up 
    glDeleteProgram(shader);
    // TODO: destory buffers HERE
    glfwDestroyWindow(window);
    glfwTerminate();

} /* end of main.c */
