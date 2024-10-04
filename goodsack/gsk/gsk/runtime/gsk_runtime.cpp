/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gsk_runtime.hpp"

#include <cstdlib>
#include <cstring>

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/device/device.h"
#include "core/drivers/alsoft/alsoft.h"
#include "entity/ecs.h"
#include "entity/lua/eventstore.hpp"
#include "wrapper/lua/lua_init.hpp"

#include <GoodsackEngineConfig.h> // TODO: change this

#if GSK_RUNTIME_USE_DEBUG
#include "tools/debug/debug_toolbar.hpp"
#endif // GSK_RUNTIME_USE_DEBUG

#ifdef RENDERER_2
#include "core/graphics/renderer/renderer.hpp"
#else
#include "core/graphics/renderer/v1/renderer.h"
#endif

#include "asset/asset_cache.h"

#include "core/drivers/alsoft/alsoft.h"

extern "C" {
static struct
{
    gsk_ECS *ecs;
    gsk_Renderer *renderer;
    gsk_AssetCache *asset_cache;

#if GSK_RUNTIME_USE_DEBUG
    gsk::tools::DebugToolbar *p_debug_toolbar;
#endif // GSK_RUNTIME_DEBUG

} s_runtime;
}

static void
_gsk_check_args(int argc, char *argv[])
{
    if (argc <= 1)
    {
        gsk_device_setGraphics(GRAPHICS_API_OPENGL);
        return;
    }

    for (int i = 0; i < argc; i++)
    {
        if (std::string(argv[i]) == "--vulkan")
        {
            gsk_device_setGraphics(GRAPHICS_API_VULKAN);
        } else if (std::string(argv[i]) == "--opengl")
        {
            gsk_device_setGraphics(GRAPHICS_API_OPENGL);
        } else if (std::string(argv[i]) == "--errlevel")
        {
            logger_setLevel(LogLevel_ERROR);
        }
    }
}

static void
_gsk_runtime_cache_asset_file(const char *uri)
{
    gsk_asset_cache_add_by_ext(s_runtime.asset_cache, uri);
}

u32
gsk::runtime::rt_setup(const char *root_dir,
                       const char *root_scheme,
                       int argc,
                       char *argv[])
{
    /*==== Initialize Logger =========================================*/

    int logStat = logger_initConsoleLogger(NULL);
    // logger_initFileLogger("logs.txt", 0, 0);

    logger_setLevel(LogLevel_TRACE);
    logger_setDetail(LogDetail_SIMPLE);

    if (logStat != 0) { LOG_INFO("Initialized Console Logger"); }
    LOG_INFO("Root directory: %s", root_dir);

    _gsk_check_args(argc, argv);

    switch (gsk_device_getGraphics())
    {
    case GRAPHICS_API_OPENGL: LOG_INFO("Device API is OpenGL"); break;
    case GRAPHICS_API_VULKAN: LOG_INFO("Device API is Vulkan"); break;
    default: LOG_ERROR("Device API Failed to retreive Graphics Backend"); break;
    }

    /*==== Setup gsk filesystem (uri) ================================*/

    gsk_filesystem_initialize(root_dir, root_scheme);

    /*==== Initialize Asset System ===================================*/

    gsk_AssetCache *p_cache = (gsk_AssetCache *)malloc(sizeof(gsk_AssetCache));
    *p_cache                = gsk_asset_cache_init();
    s_runtime.asset_cache   = p_cache;

    gsk_filesystem_traverse(_GOODSACK_FS_DIR_DATA,
                            _gsk_runtime_cache_asset_file);

    gsk_filesystem_traverse(root_dir, _gsk_runtime_cache_asset_file);

    /*==== Initialize Renderer =======================================*/

#ifdef RENDERER_2
    gsk_Renderer renderer = new gsk_Renderer();
    // ECSManager ecs = gsk_Renderer.
    gsk_Scene scene0 = renderer->SetActiveScene(0);
#else
    s_runtime.renderer = gsk_renderer_init();

    int winWidth  = s_runtime.renderer->windowWidth;
    int winHeight = s_runtime.renderer->windowHeight;

    /*==== Initialize ECS ============================================*/

    s_runtime.ecs = gsk_renderer_active_scene(s_runtime.renderer, 0);

#if 0
    // Lighting information
    vec3 lightPos   = {-3.4f, 2.4f, 1.4f};
    vec4 lightColor = {0.73f, 0.87f, 0.91f, 1.0f};

    // create directional light
    gsk_lighting_add_light(&s_runtime.renderer->lighting_data,
                           (float *)lightPos,
                           (float *)lightColor);
#endif

#if GSK_RUNTIME_USE_DEBUG

    /*==== Initialize Debug Toolbar ==================================*/

    s_runtime.p_debug_toolbar =
      new gsk::tools::DebugToolbar(s_runtime.renderer);

#endif // GSK_RUNTIME_USE_DEBUG

    /*==== Runtime setup =============================================*/

    // FPS Counter
    gsk_device_resetTime();

    // Initialize Graphics Settings
    gsk_device_setGraphicsSettings((gsk_GraphicsSettings {.swapInterval = 1}));
    // Initialize gsk_Input
    gsk_device_setInput((gsk_Input {.cursor_position = {0, 0}}));
    device_setCursorState(INIT_CURSOR_LOCKED, INIT_CURSOR_VISIBLE);

    // Initialize audio interface
    openal_init();

#if USING_RUNTIME_LOADING_SCREEN
    glfwSwapInterval(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    gsk_GuiText *loading_text = gsk_gui_text_create("Loading");
    for (int i = 0; i < 2; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        gsk_gui_text_draw(loading_text);
        glfwSwapBuffers(s_runtime.renderer->window); // we need to swap.
    }
#endif // RUNTIME_LOADING_SCREEN

#ifdef USING_LUA
    // Main Lua entry
    // TODO: possibly refactor this path (make mutable)
    char path[GSK_FS_MAX_PATH];
    strcpy(path, root_scheme);
    strcat(path, "://scripts/main.lua");
    LuaInit(GSK_PATH(path), s_runtime.ecs);
#endif

#endif
    return 0;
}

void
gsk::runtime::rt_loop()
{
    gsk_renderer_start(
      s_runtime.renderer); // Initialization for the render loop

#if USING_LUA

    // Register components in Lua ECS
    // TODO: automate

    entity::LuaEventStore::GetInstance().m_ecs = s_runtime.ecs;

    entity::LuaEventStore::GetInstance().RegisterComponentList(C_CAMERA,
                                                               "Camera");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_CAMERALOOK,
                                                               "CameraLook");
    entity::LuaEventStore::GetInstance().RegisterComponentList(
      C_CAMERAMOVEMENT, "CameraMovement");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_TRANSFORM,
                                                               "Transform");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_WEAPON,
                                                               "Weapon");
    entity::LuaEventStore::GetInstance().RegisterComponentList(C_WEAPONSWAY,
                                                               "WeaponSway");

    // ECS Lua Init
    entity::LuaEventStore::ECSEvent(ECS_INIT); // TODO: REMOVE
#endif

    // Main Engine Loop
    while (!glfwWindowShouldClose(s_runtime.renderer->window))
    {
        gsk_device_updateTime(glfwGetTime());

#if USING_JOYSTICK_CONTROLLER
        int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
        if (present)
        {
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

        if (GSK_DEVICE_API_OPENGL)
        {

#if USING_LUA
            entity::LuaEventStore::ECSEvent(ECS_UPDATE);
#endif // USING_LUA

            gsk_renderer_tick(s_runtime.renderer);

#if GSK_RUNTIME_USE_DEBUG
            s_runtime.p_debug_toolbar->render();
#endif // GSK_RUNTIME_USE_DEBUG

            glfwSwapBuffers(s_runtime.renderer->window); // we need to swap.
        } else if (GSK_DEVICE_API_VULKAN)
        {
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

    //
    // Cleanup
    //

    if (GSK_DEVICE_API_VULKAN)
    {
        vkDeviceWaitIdle(s_runtime.renderer->vulkanDevice->device);
        vulkan_device_cleanup(s_runtime.renderer->vulkanDevice);
    }

#if GSK_RUNTIME_USE_DEBUG
    delete (s_runtime.p_debug_toolbar);
#endif // GSK_RUNTIME_USE_DEBUG

    // TODO: asset_cache cleanup

    // cleanup audio driver
    openal_cleanup();

    glfwTerminate();
}

gsk_ECS *
gsk::runtime::rt_get_ecs()
{
    return s_runtime.ecs;
}
gsk_Renderer *
gsk::runtime::rt_get_renderer()
{
    return s_runtime.renderer;
}
gsk_AssetCache *
gsk::runtime::rt_get_asset_cache()
{
    return s_runtime.asset_cache;
}

void
gsk::runtime::rt_set_scene(u16 sceneIndex)
{
    s_runtime.ecs = gsk_renderer_active_scene(s_runtime.renderer, sceneIndex);
}
