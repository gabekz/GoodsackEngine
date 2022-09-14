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
#include "material.h"
#include "mesh.h"
#include "texture.h"

#include "primitives.h"
#include "camera.h"

#include "loaders/loader_obj.h"
#include "renderer/postbuffer.h"

static int winWidth  = DEFAULT_WINDOW_WIDTH;
static int winHeight = DEFAULT_WINDOW_HEIGHT;

#define MESH_ABSTRACTION

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
   glfwSetKeyCallback(window, _key_callback);

   // Get current OpenGL version
   printf("%s\n", glGetString(GL_VERSION));

   // Refresh rate 
   glfwSwapInterval(1);

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

// Initialize the Post Processing Framebuffer
    postbuffer_init(winWidth, winHeight);

#if 0
// --- Option 1
    Texture *textureDiff = texture_create("../res/textures/bricks-diff.png");
    Texture *textureSpec = texture_create("../res/textures/bricks-spec.png");
    Texture *textureNorm = texture_create("../res/textures/bricks-norm.png");
    ShaderProgram *ss = shader_program_create("../res/shader/lit-diffuse.shader");

    Material myMaterial = material_create(shaderCode,
        textureDiffuse, textureSpecular, textureNormalMap);
    Mesh *myMesh = mesh_create(myMaterial, "../res/models/suzanne.obj");

// --- Option 2
    ShaderProgram *ss = shader_program_create("../res/shader/white-diffuse.shader");
    Material *myMaterial = material_create(shaderCode);
    Mesh *myMesh = mesh_create_primitive(myMaterial, PRIMITIVE_PLANE);

// --- Option 3
    Material *myMaterial = material_create("../res/shaders/white-diffuse.shader");
    Mesh *myMesh = mesh_create_primitive(myMaterial, PRIMITIVE_CUBE);

    // down in render..
    mesh_draw(myMesh);
    // or
    mesh_draw_instanced(myMesh, 1000);
#endif

#if 1
// Create an object with the OBJ Loader
    VAO *vaoSuzanne = load_obj("../res/models/suzanne.obj");
    // Create texture
    Texture *tex =
        texture_create( "../res/textures/bricks.png");
    // Shader
    ShaderProgram *shaderSuzanne =
        shader_create_program("../res/shaders/lit-diffuse.shader");
    shader_use(shaderSuzanne);
    // Send texture location to shader
    //glUniform1i(glGetUniformLocation(shaderSuzanne, "u_Texture"), 0);
#else
    Texture *tex = texture_create("../res/textures/bricks.png");
    Material *matSuzanne =
        material_create("../res/shaders/lit-diffuse.shader", tex);
    Mesh *meshSuzanne = mesh_create(material, "../res/models/suzanne.obj");
#endif

#ifdef MESH_ABSTRACTION
    ShaderProgram *shaderLight = shader_create_program("../res/shaders/white.shader");
    Material *matLight = material_create(shaderLight, 0); 
    Mesh *meshLight    = mesh_create_primitive(matLight, PRIMITIVE_CUBE);
#else
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
    ShaderProgram *shaderLightObj = shader_create_program("../res/shaders/white.shader");
#endif

// Clearing GL State
    clearGL();

// Rotation parametes for the render loop
    float  rotation     = 0.0f;
    float  rotationInc  = 0.5f;
    double timePrev     = -1.0f;

// Render Loop
    while(!glfwWindowShouldClose(window)) {
    // Bind the Framebuffer
        postbuffer_bind();

    // rotation logic for model 
        if((glfwGetTime() - timePrev) >= (1.0f / 60.0f)) {
            rotation += rotationInc;
        }

    // Update the view and projection based on the camera data
        camera_send_matrix(camera, 45.0f, 0.1f, 100.0f);

#if 0
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        glm_rotate(model, glm_rad(rotation), (vec3){0.0f, 1.0f, 0.0f});
        shader_uniform(meshSuzanne->shader, "u_Model", (float *)model);
        mesh_draw(meshSuzanne);
#else
    // Drawing our first object
        shader_use(shaderSuzanne);
        // Calculating model
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        glm_rotate(model, glm_rad(rotation), (vec3){0.0f, 1.0f, 0.0f});
        int modelLoc = glGetUniformLocation(shaderSuzanne->id, "u_Model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)model);
        // Bind and draw
        vao_bind(vaoSuzanne);
        texture_bind(tex, 0);
        glDrawArrays(DRAWING_MODE, 0, 23232);
#endif

#ifdef MESH_ABSTRACTION
        shader_use(shaderLight);
        mat4 model2 = GLM_MAT4_IDENTITY_INIT;
        glm_translate(model2, lightPos);
        glUniformMatrix4fv(
          glGetUniformLocation(matLight->shaderProgram->id, "u_Model"),
          1, GL_FALSE, (float *)model2);
        //shader_uniform(meshLight->shader, "u_Model", (float *)model2);
        //material_uniform(matLight, "u_Model", TYPE_4FV, (float *)model2);
        mesh_draw(meshLight);
#else
    // drawing our second object
        shader_use(shaderLightObj);
        // Calculating model
        mat4 model2 = GLM_MAT4_IDENTITY_INIT;
        glm_translate(model2, lightPos);
        glUniformMatrix4fv(glGetUniformLocation(shaderLightObj->id, "u_Model"),
          1, GL_FALSE, (float *)model2);
        // Bind and draw
        vao_bind(vaoLight);
        glDrawArrays(DRAWING_MODE, 0, 24);
#endif

// Second pass [Post Processing is drawn to backbuffer]
    postbuffer_draw();

// Swap backbuffer + poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();
        camera_input(camera, window);
    }

// Clean-up 
    glDeleteProgram(shaderSuzanne->id);
#ifdef MESH_ABSTRACTION 
    free(shaderLight);
    free(matLight);
    free(meshLight);
#else
    glDeleteProgram(shaderLightObj->id);
#endif
    // TODO: destory buffers HERE
    glfwDestroyWindow(window);
    glfwTerminate();

} /* end of main.c */
