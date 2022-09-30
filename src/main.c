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

#include <util/debug.h>
#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <components/transform.h>
#include <components/mesh.h>
#include <components/camera.h>

#include <core/ecs.h>
#include <core/lighting.h>
#include <core/renderer.h>
#include <core/texture.h>

#include <model/material.h>

#define texture_create_d(x) texture_create(x, GL_SRGB8, true, 16)
#define texture_create_n(x) texture_create(x, GL_RGB8, false, 0)

//#define ECS_LIGHT_ENABLE

/* ~~~ MAIN ~~~ */
int main(void) {
// Renderer initialization
    Renderer *renderer = renderer_init();
    int winWidth = renderer->windowWidth;
    int winHeight = renderer->windowHeight;
    ECS *ecs; // set by active scene

#ifdef ECS_LIGHT_ENABLE
    Entity sun = ecs_new(ecs);
    ecs_add(sun, C_LIGHT, ((struct ComponentLight) {
        .type  = DirectionalLight;
        .color = (vec4){1.0f, 1.0f, 1.0f, 1.0f};
    }));
#else
// Lighting information
    float* lightPos     = (vec3){0.0f, 0.1f, 0.4f};
    float* lightColor   = (vec4){1.0f, 1.0f, 1.0f, 1.0f};

// UBO Lighting
    lighting_initialize(lightPos, lightColor);
#endif

// Textures
    Texture *texBrickDiff =
        texture_create_d("../res/textures/brickwall/diffuse.png");
    Texture *texBrickNorm =
        texture_create_n("../res/textures/brickwall/normal.png");

    Texture *texContDiff = texture_create_d("../res/textures/container/diffuse.png");
    Texture *texContSpec = texture_create_n("../res/textures/container/specular.png");

    Texture *texEarthDiff = texture_create_d("../res/textures/earth/diffuse.png");
    Texture *texEarthNorm = texture_create_n("../res/textures/earth/normal.png");

    // default textures
    Texture *texDefNorm =
        texture_create_n("../res/textures/defaults/normal.png");
    Texture *texDefSpec =
        texture_create_n("../res/textures/defaults/specular.png");

/*------------------------------------------- 
|   Scene #1
*/  ecs = renderer_active_scene(renderer, 0);

    Material *matSuzanne =
        material_create(NULL, "../res/shaders/lit-diffuse.shader", 3,
        texEarthDiff, texEarthNorm, texDefSpec);

// Create the Camera, containing starting-position and up-axis coords.
    Entity camera = ecs_new(ecs);
    ecs_add(camera, C_CAMERA, ((struct ComponentCamera) {
        .position = {0.0f, 0.0f, 2.0f},
        .axisUp   = {0.0f, 1.0f, 0.0f},
        .speed    = 0.05f,
    }));

    Entity suzanneObject = ecs_new(ecs);
    ecs_add(suzanneObject, C_TRANSFORM);
    ecs_add(suzanneObject, C_MESH, ((struct ComponentMesh) {
        .material = matSuzanne,
        .modelPath = "../res/models/sphere.obj",
        .properties = {
            .drawMode = DRAW_ARRAYS,
            .cullMode = CULL_CW | CULL_FORWARD,
        }
    }));

/*------------------------------------------- 
|   Scene #2
*/
    ecs = renderer_active_scene(renderer, 1);

    Material *matFloor = 
        material_create(NULL, "../res/shaders/lit-diffuse.shader",
        3, texBrickDiff, texBrickNorm, texDefSpec); 
    Material *matBox = 
        material_create(NULL, "../res/shaders/lit-diffuse.shader",
        3, texContDiff, texDefNorm, texContSpec); 

    Entity camera2 = ecs_new(ecs);
    ecs_add(camera2, C_CAMERA, ((struct ComponentCamera) {
        .position = {0.0f, 0.0f, 2.0f},
        .axisUp   = {0.0f, 1.0f, 0.0f},
        .speed    = 0.05f,
    }));

    Entity floorEntity = ecs_new(ecs);
    ecs_add(floorEntity, C_TRANSFORM, ((struct ComponentTransform) {
            .position = {0.0f, -0.3f, 0.0f},
            .scale = {10.0f, 10.0f, 10.0f},
        }));
    ecs_add(floorEntity, C_MESH, ((struct ComponentMesh) {
        .material = matFloor,
        .modelPath = "../res/models/plane.obj",
        .properties = {
            .drawMode = DRAW_ARRAYS,
            .cullMode = CULL_CW | CULL_FORWARD,
        }
    }));

    Entity boxEntity = ecs_new(ecs);
    ecs_add(boxEntity , C_TRANSFORM, ((struct ComponentTransform) {
            .position = {0.0f, -0.085f, 0.0f},
        }));
    ecs_add(boxEntity, C_MESH, ((struct ComponentMesh) {
        .material = matBox,
        .modelPath = "../res/models/cube-test.obj",
        .properties = {
            .drawMode = DRAW_ARRAYS,
            .cullMode = CULL_CW | CULL_FORWARD,
        }
    }));

/*------------------------------------------- 
|   Render Loop
*/
    renderer_active_scene(renderer, 1);
    renderer_tick(renderer);

    glfwDestroyWindow(renderer->window);
    free(renderer);
    free(ecs);
    glfwTerminate();

} /* end of main.c */
