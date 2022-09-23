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

#include <lighting/lighting.h>

#include "loaders/loader_obj.h"
#include "renderer/postbuffer.h"

#include <ecs/ecs.h>

#define USE_ECS

#define texture_create_d(x) texture_create(x, GL_SRGB8, true, 16.0f)
#define texture_create_n(x) texture_create(x, GL_RGB8, false, 0.0f)

/* ~~~ MAIN ~~~ */
int main(void) {
// Renderer initialization
    Renderer *renderer = renderer_init();
    int winWidth = renderer->windowWidth;
    int winHeight = renderer->windowHeight;

#ifdef USE_ECS
/*------------------------------------------- 
|   ECS Testing
*/
    // Initialize ECS and all ECS Systems.
    ECS *ecs = ecs_init();

    // define a component
    struct ComponentTest {
        ui32 x, y;
    };
    ecs_component_register(ecs, C_TEST, sizeof(struct ComponentTest));

    // Test entity
    Entity test = ecs_new(ecs);
    ecs_add(test, C_TEST, ((struct ComponentTest) {
        .x = 500,
        .y = 12,
    }));
    struct ComponentTest *testCopyReal = ecs_get(test, C_TEST);

    //(*(char *)(ecs->component_lists[C_TEST].components+size)) = 0; SET TAG
    printf("\ndata: %d, %d.", testCopyReal->x, testCopyReal->y /**/);
        // tag... *((char *)testCopyReal+size) & 0xff);

    printf("\n entity has C_TEST?: %d ", ecs_has(test, C_TEST));

    ecs_event(ecs, ECS_INIT);

#endif


// Create the Camera, containing starting-position and up-axis coords.
    // TODO: Update camera when window is resized
    Camera* camera = camera_create(winWidth, winHeight,
      (vec3){0.0f, 0.0f, 2.0f}, (vec3){0.0f, 1.0f, 0.0f});

// Lighting information
    float* lightPos     = (vec3){0.0f, 0.1f, 0.4f};
    float* lightColor   = (vec4){1.0f, 1.0f, 1.0f, 1.0f};

// UBO Lighting
    lighting_initialize(lightPos, lightColor);

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
|   Scene #1 Objects
*/  renderer_active_scene(renderer, 0);

    Material *matSuzanne =
        material_create(NULL, "../res/shaders/lit-diffuse.shader", 3,
        texEarthDiff, texEarthNorm, texDefSpec);
    Mesh *meshSuzanne =
        mesh_create_obj(matSuzanne , "../res/models/sphere.obj", 1.0f,
            1, GL_FRONT, GL_CW);
    // Update transform
    mat4 suzanneT = GLM_MAT4_IDENTITY_INIT;
    glm_translate(suzanneT, (vec3){0.0f, 0.0f, 0.0f});
    mesh_set_matrix(meshSuzanne, suzanneT);

// Send models to the renderer
    renderer_add_mesh(renderer, meshSuzanne);

/*------------------------------------------- 
|   Scene #2 Objects
*/  renderer_active_scene(renderer, 1);

    Material *matFloor = 
        material_create(NULL, "../res/shaders/lit-diffuse.shader",
        3, texBrickDiff, texBrickNorm, texDefSpec); 
    Mesh *meshFloor =
        mesh_create_obj(matFloor, "../res/models/plane.obj", 10.00f, 0, 0, 0);
    // Send transform to mesh->model and shader
    mat4 floorT = GLM_MAT4_IDENTITY_INIT;
    glm_translate(floorT, (vec3){0.0f, -0.3f, 0.0f});
    mesh_set_matrix(meshFloor, floorT);

// Create the box mesh
    Material *matBox = 
        material_create(NULL, "../res/shaders/lit-diffuse.shader",
        3, texContDiff, texDefNorm, texContSpec); 
    Mesh *meshBox =
        mesh_create_obj(matBox, "../res/models/cube-test.obj",
            1.0f, 0, 0, 0);
    // Send transform to mesh->model and shader
    mat4 boxT = GLM_MAT4_IDENTITY_INIT;
    glm_translate(boxT, (vec3){0.0f, -0.085f, 0.0f});
    mesh_set_matrix(meshBox, boxT);

// Add meshes to scene
    renderer_add_mesh(renderer, meshBox);
    renderer_add_mesh(renderer, meshFloor);

/*------------------------------------------- 
|   Render Loop
*/
    renderer_active_scene(renderer, 1);
    renderer_tick(renderer, camera);

    glfwDestroyWindow(renderer->window);
    free(renderer);
    free(ecs);
    glfwTerminate();

} /* end of main.c */
