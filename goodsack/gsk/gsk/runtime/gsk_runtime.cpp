/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gsk_runtime.hpp"

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/device/device.h"
#include "core/graphics/lighting/lighting.h"
#include "entity/lua/eventstore.hpp"
#include "entity/v1/ecs.h"
#include "wrapper/lua/lua_init.hpp"

#if GSK_RUNTIME_USE_DEBUG
#include "tools/debug/debug_toolbar.hpp"
#endif // GSK_RUNTIME_USE_DEBUG

#include "entity/v1/builtin/component_test.h"

#ifdef RENDERER_2
#include "core/graphics/renderer/renderer.hpp"
#else
#include "core/graphics/renderer/v1/renderer.h"
#endif

extern "C" {
static struct
{
    gsk_ECS *ecs;
    gsk_Renderer *renderer;

#if GSK_RUNTIME_USE_DEBUG
    gsk::tools::DebugToolbar *p_debug_toolbar;
#endif // GSK_RUNTIME_DEBUG

} s_runtime;
}

static void
_gsk_check_args(int argc, char *argv[])
{
    if (argc <= 1) {
        device_setGraphics(GRAPHICS_API_OPENGL);
        return;
    }

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

ui32
gsk_runtime_setup(const char *root_dir, int argc, char *argv[])
{
    // Setup logger
    int logStat = logger_initConsoleLogger(NULL);
    // logger_initFileLogger("logs/logs.txt", 0, 0);

    logger_setLevel(LogLevel_NONE);
    logger_setDetail(LogDetail_SIMPLE);

    if (logStat != 0) { LOG_INFO("Initialized Console Logger"); }

    _gsk_check_args(argc, argv);

    switch (device_getGraphics()) {
    case GRAPHICS_API_OPENGL: LOG_INFO("Device API is OpenGL"); break;
    case GRAPHICS_API_VULKAN: LOG_INFO("Device API is Vulkan"); break;
    default: LOG_ERROR("Device API Failed to retreive Graphics Backend"); break;
    }

    gsk_filesystem_initialize(root_dir);

    // Initialize Renderer
#ifdef RENDERER_2
    gsk_Renderer renderer = new gsk_Renderer();
    // ECSManager ecs = gsk_Renderer.
    gsk_Scene scene0 = renderer->SetActiveScene(0);
#else
    s_runtime.renderer = gsk_renderer_init();

    int winWidth  = s_runtime.renderer->windowWidth;
    int winHeight = s_runtime.renderer->windowHeight;

    // Initialize ECS
    s_runtime.ecs = gsk_renderer_active_scene(s_runtime.renderer, 0);

    // Lighting information
    vec3 lightPos   = {1.5f, 2.4f, -0.5f};
    vec4 lightColor = {0.95f, 0.87f, 0.78f, 1.0f};

    // UBO Lighting
    s_runtime.renderer->light =
      gsk_lighting_initialize((float *)lightPos, (float *)lightColor);

#if GSK_RUNTIME_USE_DEBUG
    // Create DebugToolbar
    s_runtime.p_debug_toolbar =
      new gsk::tools::DebugToolbar(s_runtime.renderer);
#endif // GSK_RUNTIME_USE_DEBUG

    // FPS Counter
    device_resetAnalytics();

    // Initialize Graphics Settings
    device_setGraphicsSettings((GraphicsSettings {.swapInterval = 1}));
    // Initialize Input
    device_setInput((Input {.cursor_position = {0, 0}}));
    device_setCursorState(INIT_CURSOR_LOCKED, INIT_CURSOR_VISIBLE);

#if USING_RUNTIME_LOADING_SCREEN
    glfwSwapInterval(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    gsk_GuiText *loading_text = gsk_gui_text_create("Loading");
    for (int i = 0; i < 2; i++) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        gsk_gui_text_draw(loading_text);
        glfwSwapBuffers(s_runtime.renderer->window); // we need to swap.
    }
#endif // RUNTIME_LOADING_SCREEN

#ifdef USING_LUA
    // Main Lua entry
    // TODO: possibly refactor this path (make mutable)
    LuaInit(GSK_PATH("data://scripts/main.lua"), s_runtime.ecs);
#endif

#endif
    return 0;
}

void
gsk_runtime_loop()
{
    gsk_renderer_start(s_runtime.renderer); // Initialization for the render loop

#if USING_LUA

    // Register components in Lua ECS

    entity::LuaEventStore::GetInstance().m_ecs = s_runtime.ecs;

    entity::LuaEventStore::GetInstance().RegisterComponentList(C_CAMERA,
                                                               "Camera");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_CAMERALOOK,
                                                               "CameraLook");
    entity::LuaEventStore::GetInstance().RegisterComponentList(
      C_CAMERAMOVEMENT, "CameraMovement");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_TRANSFORM,
                                                               "Transform");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_TEST, "Test");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_WEAPON,
                                                               "Weapon");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_WEAPONSWAY,
                                                               "WeaponSway");

    // ECS Lua Init
    entity::LuaEventStore::ECSEvent(ECS_INIT); // TODO: REMOVE
#endif

    // Main Engine Loop
    while (!glfwWindowShouldClose(s_runtime.renderer->window)) {
        device_updateAnalytics(glfwGetTime());

#if USING_JOYSTICK_CONTROLLER
        int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
        if (present) {
            {
                int count;
                const float *axes =
                  glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

                LOG_INFO("Axes0 %d: %f", 0, axes[0]);
                LOG_INFO("Axes1 %d: %f", 1, axes[1]);
            }
            {
                int count;
                const unsigned char *buttons =
                  glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);

                if (buttons[1] == GLFW_PRESS) { LOG_INFO("Press"); }
            }
        }
#endif

        if (DEVICE_API_OPENGL) {

#if USING_LUA
            entity::LuaEventStore::ECSEvent(ECS_UPDATE);
#endif // USING_LUA

            gsk_renderer_tick(s_runtime.renderer);

#if GSK_RUNTIME_USE_DEBUG
            s_runtime.p_debug_toolbar->render();
#endif // GSK_RUNTIME_USE_DEBUG

            glfwSwapBuffers(s_runtime.renderer->window); // we need to swap.
        } else if (DEVICE_API_VULKAN) {
            glfwPollEvents();
            // debugGui->Update();
            gsk_ecs_event(s_runtime.ecs, ECS_UPDATE); // TODO: REMOVE
            vulkan_render_draw_begin(s_runtime.renderer->vulkanDevice,
                                     s_runtime.renderer->window);
            s_runtime.renderer->currentPass = REGULAR;
            gsk_ecs_event(s_runtime.ecs, ECS_RENDER);

#if GSK_RUNTIME_USE_DEBUG
            s_runtime.p_debug_toolbar->render();
#endif // GSK_RUNTIME_USE_DEBUG

            vulkan_render_draw_end(s_runtime.renderer->vulkanDevice,
                                   s_runtime.renderer->window);
        }
    }

    LOG_TRACE("Closing Application");

    // Cleanup
    if (DEVICE_API_VULKAN) {
        vkDeviceWaitIdle(s_runtime.renderer->vulkanDevice->device);
        vulkan_device_cleanup(s_runtime.renderer->vulkanDevice);
    }

#if GSK_RUNTIME_USE_DEBUG
    delete (s_runtime.p_debug_toolbar);
#endif // GSK_RUNTIME_USE_DEBUG

    glfwTerminate();
}

gsk_ECS *
gsk_runtime_get_ecs()
{
    return s_runtime.ecs;
}
gsk_Renderer *
gsk_runtime_get_renderer()
{
    return s_runtime.renderer;
}

void
gsk_runtime_set_scene(ui16 sceneIndex)
{
    s_runtime.ecs = gsk_renderer_active_scene(s_runtime.renderer, sceneIndex);
}
