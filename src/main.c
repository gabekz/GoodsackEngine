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

    renderer_active_scene(renderer, 0);

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

// Creating a set of textures
    Texture *tex = texture_create("../res/textures/bricks.png");

    Texture *texBrickDiff = texture_create("../res/textures/brickwall/diffuse.png");
    Texture *texBrickNorm = texture_create("../res/textures/brickwall/normal.png");

    Texture *texContDiff = texture_create("../res/textures/container/diffuse.png");
    Texture *texContSpec = texture_create("../res/textures/container/specular.png");

    Texture *texEarthDiff = texture_create("../res/textures/earth/diffuse.png");
    Texture *texEarthNorm = texture_create("../res/textures/earth/normal.png");

    //Texture *texWoodDiff = texture_create("../res/textures/wood/diffuse.png");
    //Texture *texWoodSpec = texture_create("../res/textures/wood/normal.png");

    // defaults
    Texture *texDefNorm =
        texture_create("../res/textures/defaults/normal.png");


// Create the suzanne object
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

// Update transform for material
//    transform_position(shaderSuzanne, (vec3){0.0f, 0.1f, 0.4f});

// Send models to the renderer
    renderer_add_mesh(renderer, meshSuzanne);
    //renderer_add_mesh(renderer, meshLight);

    renderer_active_scene(renderer, 1);

    ShaderProgram *shaderFloor=
        shader_create_program("../res/shaders/lit-diffuse.shader");
    Material *matFloor = material_create(shaderFloor, 2, texBrickDiff, texBrickNorm); 
    Mesh *meshFloor =
        mesh_create_obj(matFloor, "../res/models/plane.obj", 10.00f, 0, 0, 0);

    shader_use(shaderFloor);
    mat4 floorT = GLM_MAT4_IDENTITY_INIT;
    glm_translate(floorT, (vec3){0.0f, -0.3f, 0.0f});
    //glm_rotate(floorT, glm_rad(90.0f), (vec3){1.0f, 0.0f, 0.0f});
    glUniformMatrix4fv(
        glGetUniformLocation(shaderFloor->id, "u_Model"),
        1, GL_FALSE, (float *)floorT);

    ShaderProgram *shaderBox=
        shader_create_program("../res/shaders/lit-diffuse.shader");
    Material *matBox = 
        material_create(shaderBox, 3, texContDiff, texDefNorm, texContSpec); 
    Mesh *meshBox =
        mesh_create_obj(matBox, "../res/models/cube-triangulated.obj",
                        0.70f, 0, 0, 0);
    shader_use(shaderBox);
    mat4 boxT = GLM_MAT4_IDENTITY_INIT;
    glm_translate(boxT, (vec3){0.0f, 0.0f, 0.0f});
    glUniformMatrix4fv(
        glGetUniformLocation(shaderBox->id, "u_Model"),
        1, GL_FALSE, (float *)boxT);

    renderer_add_mesh(renderer, meshFloor);
    renderer_add_mesh(renderer, meshBox);


    renderer_active_scene(renderer, 1);
// Render Loop
    renderer_tick(renderer, camera);

// Clean-up 
    free(tex);
    free(shaderSuzanne);
    free(matSuzanne);
    free(meshSuzanne);

    free(shaderLight);
    free(matLight);
    free(meshLight);

    glfwDestroyWindow(renderer->window);
    free(renderer);
    glfwTerminate();

} /* end of main.c */
