/*H**********************************************************************
* FILENAME :        main.c
*
* DESCRIPTION :
*       Program entry.
*
* NOTES :
*       notes 
*       notes 
*
***********************************************************************H*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <cglm/cglm.h>
#include <cglm/struct.h>

#include <util/sysdefs.h>
#include <util/debug.h>

#include <renderer/renderer.h>
#include "gfx.h" // GLFW & glad headers
#include "camera.h"

#include <glbuffer/glbuffer.h>
#include "shader.h"
#include "texture.h"

#include <model/material.h>
#include <model/mesh.h>
#include <model/primitives.h>


#include "loaders/loader_obj.h"
#include "renderer/postbuffer.h"

/* ~~~ MAIN ~~~ */

int main(void) {
// Renderer initialization
    Renderer *renderer = renderer_init();
    int winWidth = renderer->windowWidth;
    int winHeight = renderer->windowHeight;

// Create the Camera, containing starting-position and up-axis coords.
    // TODO: Update camera when window is resized
    Camera* camera = camera_create(winWidth, winHeight,
      (vec3){0.0f, 0.0f, 2.0f}, (vec3){0.0f, 1.0f, 0.0f});

// Lighting information
    float* lightPos     = (vec3){0.0f, 0.1f, 0.4f};
    float* lightColor   = (vec4){1.0f, 1.0f, 1.0f, 1.0f};

    //Light *light = light_create(lightPos, lightColor, Point);
    //free(light);

// UBO Lighting
    ui32 uboLight;
    ui32 uboLightSize = sizeof(vec3) + 4 + sizeof(vec4);
    glGenBuffers(1, &uboLight);
    glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
    glBufferData(GL_UNIFORM_BUFFER, uboLightSize, NULL, GL_DYNAMIC_DRAW);
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

// Initialize the shadowmap

// Create suzanne object
    Texture *tex = texture_create("../res/textures/bricks.png");

    Texture *texBrickDiff = texture_create("../res/textures/brickwall/diffuse.png");
    Texture *texBrickNorm = texture_create("../res/textures/brickwall/normal.png");

    Texture *texContDiff = texture_create("../res/textures/container/diffuse.png");
    Texture *texContSpec = texture_create("../res/textures/container/specular.png");

    Texture *texEarthDiff = texture_create("../res/textures/earth/diffuse.png");
    Texture *texEarthNorm = texture_create("../res/textures/earth/normal.png");

    ShaderProgram *shaderSuzanne =
        shader_create_program("../res/shaders/lit-diffuse.shader");
    Material *matSuzanne =
        material_create(shaderSuzanne, 2, texEarthDiff, texEarthNorm);

    Mesh *meshSuzanne =
        mesh_create_obj(matSuzanne , "../res/models/sphere.obj", 1.0f,
            1, GL_FRONT, GL_CW);

// Create light object
    ShaderProgram *shaderLight = shader_create_program("../res/shaders/white.shader");
    Material *matLight = material_create(shaderLight, 1, tex); 
    Mesh *meshLight =
        mesh_create_primitive(matLight, PRIMITIVE_PYRAMID, 0.03f, 0, 0, 0);
    //Mesh *meshLight    = mesh_create_obj(matLight, "../res/models/cube-triangulated.obj", 1, GL_FRONT, GL_CW);

    clearGLState();

// Rotation parametes for the render loop
    float  rotation     = 0.0f;
    float  rotationInc  = 0.5f;
    double timePrev     = -1.0f;

/*------------------------------------------- 
|   Render Loop
*/
    while(!glfwWindowShouldClose(renderer->window)) {
    /*------------------------------------------- 
    |   Pass #1 - Direction Shadowmap 
    */

    /*------------------------------------------- 
    |   Pass #2 - PostProcessing Buffer
    */
        postbuffer_bind();

    // rotation logic for model 
        if((glfwGetTime() - timePrev) >= (1.0f / 60.0f)) {
            rotation += rotationInc;
        }

    // Update the view and projection based on the camera data
        camera_send_matrix(camera, 45.0f, 0.1f, 100.0f);

    // Object
        shader_use(shaderSuzanne);
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        glm_rotate(model, glm_rad(rotation), (vec3){0.0f, 1.0f, 0.0f});
        int modelLoc = glGetUniformLocation(shaderSuzanne->id, "u_Model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)model);
        // shader_uniform(matSuzanne->shaderProgram->id, "u_Model", (float *)model);
        mesh_draw(meshSuzanne);

    // Object
        shader_use(shaderLight);
        mat4 model2 = GLM_MAT4_IDENTITY_INIT;
        glm_translate(model2, lightPos);
        glUniformMatrix4fv(
          glGetUniformLocation(matLight->shaderProgram->id, "u_Model"),
          1, GL_FALSE, (float *)model2);
        mesh_draw(meshLight);

    /*------------------------------------------- 
    |   Pass #3 - Draw to backbuffer
    */
    postbuffer_draw();

// Swap backbuffer + poll for events
        glfwSwapBuffers(renderer->window);
        glfwPollEvents();
        camera_input(camera, renderer->window);
    }

// Clean-up 
    free(tex);
    free(shaderSuzanne);
    free(matSuzanne);
    free(meshSuzanne);

    free(shaderLight);
    free(matLight);
    free(meshLight);

    glfwDestroyWindow(renderer->window);
    glfwTerminate();

} /* end of main.c */
