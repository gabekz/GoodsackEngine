#include <iostream>
#include <string>

#include <util/logger.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <components/components.h>
#include <core/api/device.h>
#include <core/lighting/lighting.h>
#include <debug/debugview.hpp>
#include <ecs/ecs.h>
#include <lua/lua_init.hpp>

extern "C" {
#include <core/renderer/v1/renderer.h>
}

#define texture_create_d(x) texture_create(x, GL_SRGB_ALPHA, true, 16, NULL)
#define texture_create_n(x) texture_create(x, GL_RGB, false, 1, NULL)

int
main(int argc, char *argv[])
{
    // Logger
    int logStat = logger_initConsoleLogger(stderr);
    logger_setLevel(LogLevel_TRACE);
    logger_setDetail(LogDetail_SIMPLE);
    if (logStat != 0) { LOG_INFO("Initialized Console Logger"); }

    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            if (std::string(argv[i]) == "--vulkan") {
                device_setGraphics(GRAPHICS_API_VULKAN);
            } else if (std::string(argv[i]) == "--opengl") {
                device_setGraphics(GRAPHICS_API_OPENGL);
            } else if (std::string(argv[i]) == "--errlevel") {
                logger_setLevel(LogLevel_ERROR);
            }
        }
    }

    // openal_init();
    // exit(1);

    // Main Lua entry
    LuaInit("../src/lua/demo/main.lua");

    switch (device_getGraphics()) {
    case GRAPHICS_API_OPENGL: LOG_INFO("Device API is OpenGL"); break;
    case GRAPHICS_API_VULKAN: LOG_INFO("Device API is Vulkan"); break;
    }

    // Initialize Renderer
    Renderer *renderer = renderer_init();
    int winWidth       = renderer->windowWidth;
    int winHeight      = renderer->windowHeight;

    // Initialize ECS
    ECS *ecs;
    ecs = renderer_active_scene(renderer, 0);

    // Lighting information
    vec3 lightPos   = {1.0f, 2.8f, -0.2f};
    vec4 lightColor = {1.0f, 1.0f, 1.0f, 1.0f};

    // UBO Lighting
    lighting_initialize(lightPos, lightColor);

    Texture *texDefSpec =
      texture_create_n("../res/textures/defaults/specular.png");
    Texture *texDefNorm =
      texture_create_n("../res/textures/defaults/normal.png");
    Texture *texPbrAo = texture_create_n("../res/textures/defaults/white.png");

    Texture *texCerbA =
      texture_create_d("../res/textures/pbr/cerberus/Cerberus_A.tga");
    Texture *texCerbN =
      texture_create_n("../res/textures/pbr/cerberus/Cerberus_N.tga");
    Texture *texCerbM =
      texture_create_n("../res/textures/pbr/cerberus/Cerberus_M.tga");
    Texture *texCerbS =
      texture_create_n("../res/textures/pbr/cerberus/Cerberus_R.tga");

    Material *matCerb = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texCerbA,
                                        texCerbN,
                                        texCerbM,
                                        texCerbS,
                                        texPbrAo);

#if 0
    Texture *texEarthDiff =
      texture_create_d("../res/textures/earth/diffuse.png");
    Texture *texEarthNorm =
      texture_create_n("../res/textures/earth/normal.png");

    Texture *texContDiff =
      texture_create_d("../res/textures/container/diffuse.png");
    Texture *texContSpec =
      texture_create_n("../res/textures/container/specular.png");

    Texture *texBrickDiff =
      texture_create_d("../res/textures/brickwall/diffuse.png");
    Texture *texBrickNorm =
      texture_create_n("../res/textures/brickwall/normal.png");

    Material *matSuzanne = material_create(NULL,
                                           "../res/shaders/lit-diffuse.shader",
                                           3,
                                           texEarthDiff,
                                           texEarthNorm,
                                           texDefSpec);

    // Create the Camera, containing starting-position and up-axis coords.
    Entity camera = ecs_new(ecs);
    ecs_add(camera,
            C_CAMERA,
            ((struct ComponentCamera) {
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));

    Entity suzanneObject = ecs_new(ecs);
    ecs_add(suzanneObject, C_TRANSFORM);
    ecs_add(suzanneObject,
            C_MESH,
            ((struct ComponentMesh) {.material   = matSuzanne,
                                     .modelPath  = "../res/models/sphere.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));

    /*-------------------------------------------
    |   Scene #2
    */
    ecs = renderer_active_scene(renderer, 1);

    Material *matFloor = material_create(NULL,
                                         "../res/shaders/lit-diffuse.shader",
                                         3,
                                         texBrickDiff,
                                         texBrickNorm,
                                         texDefSpec);
    Material *matBox   = material_create(NULL,
                                       "../res/shaders/lit-diffuse.shader",
                                       3,
                                       texContDiff,
                                       texDefNorm,
                                       texContSpec);

    Entity camera2 = ecs_new(ecs);
    ecs_add(camera2,
            C_CAMERA,
            ((struct ComponentCamera) {
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));

    Entity floorEntity = ecs_new(ecs);
    ecs_add(floorEntity,
            C_TRANSFORM,
            ((struct ComponentTransform) {
              .position = {0.0f, -0.3f, 0.0f},
              .scale    = {10.0f, 10.0f, 10.0f},
            }));
    ecs_add(floorEntity,
            C_MESH,
            ((struct ComponentMesh) {.material   = matFloor,
                                     .modelPath  = "../res/models/plane.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));

    Entity boxEntity = ecs_new(ecs);
    ecs_add(boxEntity,
            C_TRANSFORM,
            ((struct ComponentTransform) {
              .position = {0.0f, -0.085f, 0.0f},
            }));
    ecs_add(boxEntity,
            C_MESH,
            ((struct ComponentMesh) {.material  = matBox,
                                     .modelPath = "../res/models/cube-test.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));

    /*-------------------------------------------
    |   Scene #3
    */
    ecs            = renderer_active_scene(renderer, 2);
    Entity camera3 = ecs_new(ecs);
    ecs_add(camera3,
            C_CAMERA,
            ((struct ComponentCamera) {
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));

    Entity sphereEntity = ecs_new(ecs);
    ecs_add(sphereEntity,
            C_TRANSFORM,
            ((struct ComponentTransform) {
              .position = {0.0f, 0.0f, 0.0f},
            }));

    Texture *texGraniteAlbedo =
      texture_create_d("../res/textures/pbr/granite/albedo.png");
    Texture *texGraniteNormal =
      texture_create_n("../res/textures/pbr/granite/normal.png");
    Texture *texGraniteMetallic =
      texture_create_n("../res/textures/pbr/granite/metallic.png");
    Texture *texGraniteSpecular =
      texture_create_n("../res/textures/pbr/granite/roughness.png");
    Texture *texGraniteAo =
      texture_create_n("../res/textures/pbr/granite/ao.png");
    Material *matGranite = material_create(NULL,
                                           "../res/shaders/pbr.shader",
                                           5,
                                           texGraniteAlbedo,
                                           texGraniteNormal,
                                           texGraniteMetallic,
                                           texGraniteSpecular,
                                           texGraniteAo);

    Texture *texPbrAlbedo =
      texture_create_d("../res/textures/pbr/rust/albedo.png");
    Texture *texPbrNormal =
      texture_create_n("../res/textures/pbr/rust/normal.png");
    Texture *texPbrMetallic =
      texture_create_n("../res/textures/pbr/rust/metallic.png");
    Texture *texPbrSpecular =
      texture_create_n("../res/textures/pbr/rust/roughness.png");
    Material *matRust = material_create(NULL,
                                        "../res/shaders/pbr.shader",
                                        5,
                                        texPbrAlbedo,
                                        texPbrNormal,
                                        texPbrMetallic,
                                        texPbrSpecular,
                                        texPbrAo);

    ecs_add(sphereEntity,
            C_MESH,
            ((struct ComponentMesh) {.material   = matGranite,
                                     .modelPath  = "../res/models/sphere.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));
    Entity floorEntity2 = ecs_new(ecs);
    ecs_add(floorEntity2,
            C_TRANSFORM,
            ((struct ComponentTransform) {
              .position = {0.0f, -0.3f, 0.0f},
              .scale    = {10.0f, 10.0f, 10.0f},
            }));
    ecs_add(floorEntity2,
            C_MESH,
            ((struct ComponentMesh) {.material   = matFloor,
                                     .modelPath  = "../res/models/plane.obj",
                                     .properties = {
                                       .drawMode = DRAW_ARRAYS,
                                       .cullMode = CULL_CW | CULL_FORWARD,
                                     }}));
#endif

    /*-------------------------------------------
    |   Scene #4
    */
    ecs = renderer_active_scene(renderer, 3);

    Entity camera4 = ecs_new(ecs);
    ecs_add(camera4,
            C_CAMERA,
            ((struct ComponentCamera) {
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));
    ecs_add(camera4, C_AUDIO_LISTENER);

    Entity entCerb = ecs_new(ecs);
    ecs_add(entCerb,
            C_TRANSFORM,
            ((struct ComponentTransform) {
              .position = {0.0f, 0.0f, 0.0f},
              .scale    = {4.0f, 4.0f, 4.0f},
            }));
    ecs_add(
      entCerb,
      C_MESH,
      ((struct ComponentMesh) {.material  = matCerb,
                               .modelPath = "../res/models/cerberus-triang.obj",
                               .properties = {
                                 .drawMode = DRAW_ARRAYS,
                                 .cullMode = CULL_CW | CULL_FORWARD,
                               }}));
    ecs_add(
      entCerb,
      C_AUDIO_SOURCE,
      ((struct ComponentAudioSource) {.filePath = "../res/audio/opening.wav"}));

    DebugGui *debugGui = new DebugGui(renderer);

    // FPS Counter
    device_resetAnalytics();

    /* Render loop */
    renderer_active_scene(renderer, 3);

    renderer_start(renderer); // Initialization for the render loop
    while (!glfwWindowShouldClose(renderer->window)) {

        device_updateAnalytics(glfwGetTime());
        // LOG_INFO("FPS: %f", device_getAnalytics().currentFps);

        if (DEVICE_API_OPENGL) {
            renderer_tick(renderer);
            debugGui->Render();
            glfwSwapBuffers(renderer->window); // we need to swap.
        } else if (DEVICE_API_VULKAN) {
            glfwPollEvents();
            // debugGui->Update();
            ecs_event(ecs, ECS_UPDATE);

            vulkan_render_draw_begin(renderer->vulkanDevice, renderer->window);
            renderer->currentPass = REGULAR;
            ecs_event(ecs, ECS_RENDER);
            debugGui->Render();
            vulkan_render_draw_end(renderer->vulkanDevice, renderer->window);
        }
    }

    // Cleanup
    if (DEVICE_API_VULKAN) {
        vkDeviceWaitIdle(renderer->vulkanDevice->device);
        vulkan_device_cleanup(renderer->vulkanDevice);
    }

    delete (debugGui);

    glfwDestroyWindow(renderer->window);
    free(renderer);
    free(ecs);
    glfwTerminate();

    return 0;
}