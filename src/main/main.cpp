#ifdef _WIN32
 //#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
 #pragma optimize("", off)
#endif

#include <iostream>
#include <string>

#include <util/maths.h>
#include <util/sysdefs.h>

#include <core/api/device.h>
#include <core/lighting/lighting.h>
#include <ecs/ecs.h>
#include <lua/lua_init.hpp>
#include <tools/debugui.hpp>

// Demo
#include "demo_scenes.h"

extern "C" {
#include <core/renderer/v1/renderer.h>
}

int
main(int argc, char *argv[])
{
    // Logger
    int logStat = logger_initConsoleLogger(NULL);
    logger_initFileLogger("logs/logs.txt", 0, 0);

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

    // Main Lua entry
    //LuaInit("../src/lua/demo/main.lua");

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

    // create scenes
    demo_scenes_create(ecs, renderer);

    DebugGui *debugGui = new DebugGui(renderer);

    // FPS Counter
    device_resetAnalytics();

    /* Render loop */
    ecs = renderer_active_scene(renderer, 3);

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
