#include <iostream>

#include <util/sysdefs.h>
#include <util/maths.h>

extern "C" {
#include <core/renderer.h>
#include <core/lighting.h>
#include <core/ecs.h>

#include <components/transform.h>
#include <components/camera.h>
#include <components/mesh.h>
}

#define texture_create_d(x) texture_create(x, GL_SRGB8, true, 16)
#define texture_create_n(x) texture_create(x, GL_RGB8, false, 0)

int main() {
    std::cout << "Hello world!" << std::endl;

    Renderer *renderer = renderer_init();
    int winWidth = renderer->windowWidth;
    int winHeight = renderer->windowHeight;
    ECS *ecs; // set by active scene

    ecs = renderer_active_scene(renderer, 0);

// Lighting information
    float lightPos[3]     = {0.0f, 0.1f, 0.4f};
    float lightColor[4]   = {1.0f, 1.0f, 1.0f, 1.0f};

// UBO Lighting
    lighting_initialize(lightPos, lightColor);

    Texture *texEarthDiff = texture_create_d("../res/textures/earth/diffuse.png");
    Texture *texEarthNorm = texture_create_n("../res/textures/earth/normal.png");
    Texture *texDefSpec =
        texture_create_n("../res/textures/defaults/specular.png");

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

    /* Render loop */

    renderer_active_scene(renderer, 0);
    renderer_tick(renderer);

    glfwDestroyWindow(renderer->window);
    free(renderer);
    free(ecs);
    glfwTerminate();

    return 1;
}