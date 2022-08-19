/* -- main.c

   Main entry. Handling initialization.

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <cglm/cglm.h>   /* for inline */
#include <cglm/struct.h>   /* for inline */

#include "gfx.h" // GLFW & glad headers
#include "shader.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include "vao.h"
#include "vbo.h"
#include "ibo.h"
#include "texture.h"

#include "primitives.h"
#include "camera.h"

#include "loaders/loader_obj.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define NORMAL

#define RENDER_3D
#define OBJ_LOADER
#define DRAWING_MODE GL_TRIANGLES
//#define NORMALS

/*
#define VAO_NON_ABSTRACT    0
#define TEXTURE_DISABLE     0
*/

/* ~~~ CALLBACKS ~~~ */

static int vertexCount = 4;

static void _error_callback
(int error, const char* description) {
   fprintf(stderr, "Error %s\n", description);
}

static void _key_callback 
(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
   {
      //printf("Escape key was pressed");
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
   while(error)
   {
      printf("\n|OpenGL Error| (%s)\n", error);
   }
}

#ifdef CIMGUI
/* Platform */
struct ImGuiContext* ctx;
struct ImGuiIO* io;

void imgui_init(GLFWwindow* win) {
    ctx = igCreateContext(NULL);
    io = igGetIO();

    const char* glsl_version = "#version 330 core";
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);
}

#endif

/* ~~~ MAIN ~~~ */

int main(void) {
   glfwSetErrorCallback(_error_callback);
   if(!glfwInit()) { // Initialization failed
        printf("Failed to initialize glfw");         
   }

   // Minimum OpenGL version required
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
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
   
   // Get current OpenGL version
   printf("%s\n", glGetString(GL_VERSION));


#ifdef CIMGUI
   // IMGUI initialization
   imgui_init(window);
#endif

   // Refresh rate 
   glfwSwapInterval(1);

// Create the VAO [will segfault unless before vbo binding]
   VAO *vao = vao_create();
   vao_bind(vao);

#ifndef RENDER_3D
// Create a 2D Plane object
    float *positions = prim_vert_plane();
    VBO *vb = vbo_create(positions, (4 * 4) * sizeof(float));
    free(positions);

   // Create the Index Buffer for optimized vertex-position rendering
   unsigned int indices[] = {
      0, 1, 2,
      2, 3, 0
   };
    struct IBO *ib = ibo_create(indices, 6 * sizeof(unsigned int));
    // add the buffer
    vbo_push(vb, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vb, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vb);
#else
// Create a 3D Pyramid object

    float points[] =
{ //     COORDINATES     /  TexCoord   /        NORMALS       //
	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	-0.5f, 0.0f, -0.5f,     0.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	 0.5f, 0.0f, -0.5f,     5.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side

	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
	-0.5f, 0.0f, -0.5f,     5.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,     -0.8f, 0.5f,  0.0f, // Left Side

	-0.5f, 0.0f, -0.5f,     5.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
	 0.5f, 0.0f, -0.5f,     0.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.0f, 0.5f, -0.8f, // Non-facing side

	 0.5f, 0.0f, -0.5f,     0.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.8f, 0.5f,  0.0f, // Right side

	 0.5f, 0.0f,  0.5f,     5.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
	-0.5f, 0.0f,  0.5f,     0.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
	 0.0f, 0.8f,  0.0f,     2.5f, 5.0f,      0.0f, 0.5f,  0.8f  // Facing side
};

    
    VBO *vb = vbo_create(points, (3 * 2 * 3 * 16) * sizeof(float));
    //float *positions = prim_vert_pyramid();
    //VBO *vb = vbo_create(positions, ((3 * 5) + (2 * 5)) * sizeof(float));
    //free(positions);

   // Create the Index Buffer for optimized vertex-position rendering
/*
   unsigned int indices[] = {
      0, 1, 2,
      0, 2, 3,
      0, 1, 4,
      1, 2, 4,
      2, 3, 4,
      3, 0, 4,
   };
*/

    unsigned int indices[] = {
        0, 1, 2, // Bottom side
	    0, 2, 3, // Bottom side
	    4, 6, 5, // Left side
	    7, 9, 8, // Non-facing side
	    10, 12, 11, // Right side
	    13, 15, 14 // Facing side
    };

    struct IBO *ib = ibo_create(indices, (3 * 6) * sizeof(unsigned int));
    // add the buffer
    vbo_push(vb, 3, GL_FLOAT, GL_FALSE);
    vbo_push(vb, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vb, 3, GL_FLOAT, GL_FALSE); // Normals
    vao_add_buffer(vao, vb);

// Create normals?
/*
    float *normPositions = prim_norm_pyramid();
    VBO *vb_norm = vbo_create(normPositions, 3 * 4 * 4 * sizeof(float));
    free(normPositions);
    vbo_push(vb_norm, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vb_norm);
*/

#endif

// Create the point-light object
   VAO *vaoLight = vao_create();
   vao_bind(vaoLight);
// Cube for point-light
    float *lightPositions = prim_vert_cube(0.03f);
    VBO *vbLight = vbo_create(lightPositions, (3 * 8) * sizeof(float));
    free(lightPositions);

   unsigned int lightIndices[] = {
       0, 1, 2,
       0, 2, 3,
       0, 4, 7,
       0, 7, 3,
       3, 7, 6,
       3, 6, 2,
       2, 6, 5,
       2, 5, 1,
       1, 5, 4,
       1, 4, 0,
       4, 5, 6,
       4, 6, 7
   };
    struct IBO *ibLight = ibo_create(lightIndices , (3 * 12) * sizeof(unsigned int));

    vbo_push(vbLight, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vaoLight, vbLight);

#ifdef TEST_TIME
    printf("Vertex Shader:\n%s", ss->shaderVertex);
    printf("Fragment Shader:\n%s", ss->shaderFragment);
#endif

#ifdef OBJ_LOADER
// TEST: Load an OBJ file
   VAO* vaoLoaded = load_obj("../res/models/suzanne.obj");
#endif

// Set up for shaders
    MyShaderStruct *ss =
      //ParseShader("../res/shaders/unlit-textured.shader");
        ParseShader("../res/shaders/lit-diffuse.shader");

   unsigned int shader = CreateShader(ss->shaderVertex, ss->shaderFragment);
   glUseProgram(shader); // Activate the shader


    // Setup for the second shader (test)
    MyShaderStruct *ss2 = 
        ParseShader("../res/shaders/white.shader");
   unsigned int shader2 = CreateShader(ss2->shaderVertex, ss2->shaderFragment);

    /* Note: Once we've set up the shader, we can free contents */
    free(ss);
    free(ss2);

// Shader Fun 
   int location = glGetUniformLocation(shader, "u_Color");
   float r = 0.0f;
   float increment = 0.005f;

// Create texture
    struct Texture *tex = texture_create(
      (unsigned char *)"../res/textures/bricks.png");
    texture_bind(tex);
// ..send it to the active shader
    int location2 = glGetUniformLocation(shader, "u_Texture");
    glUniform1i(location2, 0); // 0 is the first (only) texture?

// Create the camera, containing position and up-axis coords
    Camera* camera = camera_create(winWidth, winHeight,
      (vec3){0.0f, 0.0f, 2.0f}, (vec3){0.0f, 1.0f, 0.0f});

// Clearing GL State
    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);            // unbind VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    // unbind IBO
    // Free VBO's [not needed anymore, the data exists in the VAO's]
    free(vb);
    free(vbLight);

// Enable Depth Testing
    glEnable(GL_DEPTH_TEST);

// Lighting information
    float* lightColor   = (vec4){1.0f, 1.0f, 1.0f, 1.0f};
    float* lightPos     = (vec3){0.0f, 0.5f, 0.4f};
    //vec4 lightColor = GLM_VEC4_ONE_INIT;
    //lightColor = (float *){1.0f, 1.0f, 1.0f, 1.0f};

    int locationLightColor  = glGetUniformLocation(shader, "u_LightColor");
    int locationLightColor2 = glGetUniformLocation(shader2, "u_LightColor");
    int locationLightPos    = glGetUniformLocation(shader, "u_LightPosition");

    glUseProgram(shader);

    glUniform3f(locationLightPos,
            (float)lightPos[0],
            (float)lightPos[1],
            (float)lightPos[2]);
    glUniform4f(locationLightColor,
            (float)lightColor[0], (float)lightColor[1],
            (float)lightColor[2], (float)lightColor[3]);

    glUseProgram(shader2);

    glUniform4f(locationLightColor2,
            (float)lightColor[0], (float)lightColor[1],
            (float)lightColor[2], (float)lightColor[3]);

// Rotation stuff for shader
    float rotation = 0.0f;
    float rotationInc = 0.5f;
    double timePrev = -1.0f;

// Render Loop
    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.15f, 1.0f);

        // Tell OpenGL which shader to use
        glUseProgram(shader);

        // color-shift logic for shader
        if(r > 1.0f || r < 0.0f) {
            increment = -increment;
        }
        r+=increment;
        glUniform4f(location, r, 1.0f, 1.0f, 1.0f);

        // rotation logic for pyramid
        int timeCurr = glfwGetTime();
        if((timeCurr - timePrev) >= (1.0f / 60.0f)) {
            rotation += rotationInc;
        }
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        glm_rotate(model, glm_rad(rotation), (vec3){0.0f, 1.0f, 0.0f});

        int modelLoc = glGetUniformLocation(shader, "model");       // Model
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)model);

        mat4 testThing = GLM_MAT4_IDENTITY_INIT;
        glm_mat4_mul(camera->proj, camera->view, testThing);
        int modelLocTest = glGetUniformLocation(shader, "u_NormMat");       // Model
        glUniformMatrix4fv(modelLocTest, 1, GL_FALSE, (float *)testThing);

    // Update the view and projection based on the camera data
        camera_send_matrix(camera, 45.0f, 0.1f, 100.0f);
        camera_matrix(camera, shader, "camMatrix"); // send to shader

    // send our camera position to the shader
    int locationCameraPosition = glGetUniformLocation(shader, "u_CamPos");
    glUniform3f(locationCameraPosition,
      camera->position[0], camera->position[1], camera->position[2]);

    // Compute camera-input
        camera_input(camera, window);

    // drawing our object
        //ibo_bind(ib); -- no need to do this if we bind the vao
        vao_bind(vao);
        //glDrawElements(DRAWING_MODE, 18, GL_UNSIGNED_INT, NULL);

    // drawing our second object
        // Tell OpenGL which shader to use
        glUseProgram(shader2);
        camera_matrix(camera, shader2, "camMatrix"); // send to shader
        mat4 model2 = GLM_MAT4_IDENTITY_INIT;
        int modelLoc2 = glGetUniformLocation(shader2, "model");       // Model

        glm_translate(model2, lightPos);
        glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, (float *)model2);

        vao_bind(vaoLight);
        glDrawElements(DRAWING_MODE, 36, GL_UNSIGNED_INT, NULL); 

#ifdef OBJ_LOADER
        vao_bind(vaoLoaded);
        glUseProgram(shader);
        //glDrawElements(GL_LINE_LOOP, vertexCount, GL_UNSIGNED_INT, NULL); 
        glDrawArrays(GL_TRIANGLES, 0, 23232);
#endif

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
