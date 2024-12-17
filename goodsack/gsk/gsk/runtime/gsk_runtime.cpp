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

#include "asset/asset.h"
#include "asset/asset_cache.h"
#include "asset/gpak/gpak.h"

#include "core/drivers/alsoft/alsoft.h"

#if GSK_USING_COMPOSER
#include "core/audio/music_composer.h"
#include "core/audio/music_composer_loader.hpp"
#endif // GSK_USING_COMPOSER

#define _TOTAL_ASSET_CACHES 2
#define _TEST_WRITE_PNG     0

extern "C" {
static struct
{
    gsk_ECS *ecs;
    gsk_Renderer *renderer;

    gsk_AssetCache *pp_asset_caches[_TOTAL_ASSET_CACHES];
    u32 cache_cnt;
    char proj_scheme[GSK_FS_MAX_SCHEME];

#if GSK_RUNTIME_USE_DEBUG
    gsk::tools::DebugToolbar *p_debug_toolbar;
#endif // GSK_RUNTIME_DEBUG

    struct
    {
        u8 fs_mode;    // 0 = gpak; 1 = hot
        u8 build_gpak; // 0 = FALSE; 1 = TRUE
    } options;

} s_runtime;
} // extern "C"

static void
_gsk_check_args(int argc, char *argv[])
{
    // If no arguments are given, default to OpenGL
    if (argc <= 1)
    {
        gsk_device_setGraphics(GRAPHICS_API_OPENGL);
        return;
    }

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        // Check for key=value arguments first
        size_t equal_pos = arg.find('=');
        if (equal_pos != std::string::npos)
        {
            std::string key   = arg.substr(0, equal_pos);
            std::string value = arg.substr(equal_pos + 1);

            // Handle "--gpu=..."
            if (key == "--gpu")
            {
                if (value == "vulkan")
                {
                    gsk_device_setGraphics(GRAPHICS_API_VULKAN);
                } else if (value == "opengl")
                {
                    gsk_device_setGraphics(GRAPHICS_API_OPENGL);
                } else
                {
                    LOG_WARN("GPU type not specified. Fallback to OpenGL");
                    gsk_device_setGraphics(GRAPHICS_API_OPENGL);
                }
            }
            // Handle "--map=..."
            else if (key == "--map")
            {
                LOG_INFO("Loading map from: %s", value.c_str());
                // TODO: implement your map loading logic here
            }
        }
        // Check flags without values
        else
        {
            if (arg == "--errlevel")
            {
                logger_setLevel(LogLevel_ERROR);
            } else if (arg == "--hot")
            {
                LOG_INFO("Set FS Mode to RunHot");
                s_runtime.options.fs_mode = 1;
            } else if (arg == "--testgpak")
            {
                LOG_INFO("Testing GPAK (overriding FS Mode to RunHot)");
                s_runtime.options.build_gpak = 1;
                s_runtime.options.fs_mode    = 1;
            }
        }
    }
}

static void
_gsk_runtime_cache_asset_file(const char *uri)
{
    gsk_asset_cache_add_by_ext(s_runtime.pp_asset_caches[s_runtime.cache_cnt],
                               uri);
}

u32
gsk::runtime::rt_setup(const char *root_dir,
                       const char *root_scheme,
                       const char *app_name,
                       int argc,
                       char *argv[])
{
    /*==== Initialize Logger =========================================*/

    int logStat = logger_initConsoleLogger(NULL);
    // logger_initFileLogger(exe_path.c_str(), 0, 0);

    logger_setLevel(LogLevel_TRACE);
    logger_setDetail(LogDetail_SIMPLE);

    if (logStat != 0) { LOG_INFO("Initialized Console Logger"); }
    LOG_INFO("Root directory: %s", root_dir);

    s_runtime.cache_cnt          = 0; // TODO: Change this.
    s_runtime.options.fs_mode    = 0; // TODO: Change this.
    s_runtime.options.build_gpak = 0; // TODO: Change this.

    _gsk_check_args(argc, argv);

    switch (gsk_device_getGraphics())
    {
    case GRAPHICS_API_OPENGL: LOG_INFO("Device API is OpenGL"); break;
    case GRAPHICS_API_VULKAN: LOG_INFO("Device API is Vulkan"); break;
    default: LOG_ERROR("Device API Failed to retreive Graphics Backend"); break;
    }

    /*==== Setup gsk filesystem (uri) ================================*/

    strcpy(s_runtime.proj_scheme, root_scheme);
    gsk_filesystem_initialize(root_dir, root_scheme);

    /*==== Initialize Asset System ===================================*/

    for (int i = 0; i < _TOTAL_ASSET_CACHES; i++)
    {
        // weird hack to stop possible issue
        if (i >= 2) { LOG_CRITICAL("Not implemented.."); }

        gsk_AssetCache *p_cache =
          (gsk_AssetCache *)malloc(sizeof(gsk_AssetCache));

        *p_cache = gsk_asset_cache_init(
          ((i == 0) ? GSK_FS_GSK_SCHEME : s_runtime.proj_scheme));
        s_runtime.pp_asset_caches[i] = p_cache;
    }

    // TODO: Setup default assets here
    // gsk_asset_cache_add(p_cache, 0, "gsk:bin//defaults/material");

    // GPAK
    if (s_runtime.options.fs_mode == 0)
    {
        const char *path = (_GOODSACK_FS_DIR_BUILD "/output/gpak/gsk.gpak");
        gsk_gpak_reader_fill_cache(s_runtime.pp_asset_caches[0], path);

#if _TEST_WRITE_PNG
        gsk_AssetBlob blob = gsk_gpak_reader_import_blob("gsk://map/Icon.png");
        LOG_INFO("%d", blob.buffer_len);

        FILE *file_test;
        file_test = fopen(GSK_PATH("gsk://test.png"), "wb");
        if (file_test == NULL) { LOG_CRITICAL("FAIL"); }
        fwrite(blob.p_buffer, 1, blob.buffer_len, file_test);
        fclose(file_test);
#endif

        exit(0);
    }
    // HOT
    else if (s_runtime.options.fs_mode == 1)
    {
        // TODO: filesystem traverse should be sorted to be platform-agnostic
        s_runtime.cache_cnt = 0;
        gsk_filesystem_traverse(_GOODSACK_FS_DIR_DATA,
                                _gsk_runtime_cache_asset_file);

        s_runtime.cache_cnt = 1;
        gsk_filesystem_traverse(root_dir, _gsk_runtime_cache_asset_file);

#if 0
        _gsk_asset_get_internal(s_runtime.pp_asset_caches[0],
                                "gsk://models/cube.obj",
                                GSK_ASSET_FETCH_IMPORT);
        exit(0);
#endif

        // NOTE: test build_gpak requires hot-loading
        if (s_runtime.options.build_gpak)
        {
            const char *path_gpak = (_GOODSACK_FS_DIR_BUILD "/output/gpak/");

            for (int i = 0; i < _TOTAL_ASSET_CACHES; i++)
            {
                gsk_GpakWriter writer =
                  gsk_gpak_writer_init(s_runtime.pp_asset_caches[i], path_gpak);

                gsk_gpak_writer_populate_cache(&writer);
                gsk_gpak_writer_close(&writer);
            }

#if GSK_TESTGPAK_EXIT
            exit(0);
#endif
        }
    }

    // preload all GCFG files per Asset Cache
    for (int i = 0; i < _TOTAL_ASSET_CACHES; i++)
    {
        gsk_AssetCache *p_cache = s_runtime.pp_asset_caches[i];
        ArrayList *p_gcfg_refs  = &(p_cache->asset_lists[0].list_state);

        for (int i = 0; i < p_gcfg_refs->list_next; i++)
        {
            gsk_AssetRef *p_ref =
              (gsk_AssetRef *)array_list_get_at_index(p_gcfg_refs, i);

            char *str;
            str = (char *)array_list_get_at_index(&(p_cache->asset_uri_list),
                                                  p_ref->asset_uri_index);

            // TODO: Do not reference by URI, reference by handle.
            GSK_ASSET(str);
        }
    }

    /*==== Initialize Renderer =======================================*/

#ifdef RENDERER_2
    gsk_Renderer renderer = new gsk_Renderer();
    // ECSManager ecs = gsk_Renderer.
    gsk_Scene scene0 = renderer->SetActiveScene(0);
#else
    s_runtime.renderer = gsk_renderer_init(app_name);

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

    vec2 text_pos   = {0.0f, 0.0f};
    vec3 text_color = {1.0f, 1.0f, 1.0f};

    gsk_GuiText *loading_text =
      gsk_gui_text_create("Loading", text_pos, text_color);

    for (int i = 0; i < 2; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        gsk_gui_text_draw(loading_text);
        glfwSwapBuffers(s_runtime.renderer->window); // we need to swap.
    }
#endif // RUNTIME_LOADING_SCREEN

#if USING_LUA
    // Main Lua entry
    // TODO: possibly refactor this path (make mutable)
    char path[GSK_FS_MAX_PATH];
    strcpy(path, root_scheme);
    strcat(path, "://scripts/main.lua");
    LuaInit(GSK_PATH(path), s_runtime.ecs);
#endif

    // const char *info =
    //  ("Goodsack Engine | " GOODSACK_VERSION_MAJOR "."
    //  GOODSACK_VERSION_MINOR);

    char str_info[256];
    sprintf(str_info,
            "Goodsack Engine | v%d.%d.%d.%d",
            GOODSACK_VERSION_MAJOR,
            GOODSACK_VERSION_MINOR,
            GOODSACK_VERSION_PATCH,
            GOODSACK_VERSION_TWEAK);

    vec2 text_info_pos = {5.0f, 5.0f};
    vec3 text_info_col = {1.0f, 1.0f, 1.0f};

    gsk_GuiText *text_info =
      gsk_gui_text_create(str_info, text_info_pos, text_info_col);
    gsk_gui_canvas_add_text(&s_runtime.renderer->canvas, text_info);

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

#if GSK_USING_COMPOSER
    gsk_MusicComposer composer = gsk_music_composer_create();
    gsk::audio::composer::create_from_json(GSK_PATH("gsk://composer.json"));
#endif // GSK_USING_COMPOSER

    // Main Engine Loop
    while (!glfwWindowShouldClose(s_runtime.renderer->window))
    {
        double time_sec = glfwGetTime();
        gsk_device_updateTime(time_sec);

#if GSK_USING_COMPOSER
        gsk_music_composer_update(&composer, time_sec);
#endif // GSK_USING_COMPOSER

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

    // Delete all ECS Entities
    for (int i = 0; i < s_runtime.renderer->sceneC; i++)
    {
        if (s_runtime.renderer->sceneL[i] == NULL) { continue; }

        gsk_ECS *p_ecs = s_runtime.renderer->sceneL[i]->ecs;

        if (p_ecs == NULL) { continue; }

        // mark each entity for deletion
        for (int j = 0; j < p_ecs->nextIndex; j++)
        {
            gsk_ecs_ent_destroy(gsk_ecs_ent(p_ecs, p_ecs->ids[j]));
        }

        // call ECS_DESTROY for each ECS handler
        gsk_ecs_event(s_runtime.renderer->sceneL[i]->ecs, ECS_DESTROY);
    }

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
gsk::runtime::rt_get_asset_cache(const char *uri_str)
{
    // validate uri
    gsk_URI uri  = gsk_filesystem_uri(uri_str);
    void *p_data = NULL;

    if (!strcmp(uri.scheme, GSK_FS_GSK_SCHEME))
    {
        return s_runtime.pp_asset_caches[0];
    } else if (!strcmp(uri.scheme, s_runtime.proj_scheme))
    {

        return s_runtime.pp_asset_caches[1];
    }
    LOG_ERROR("Failed to find asset cache for: %s", uri_str);
    return NULL;
}

void
gsk::runtime::rt_set_scene(u16 sceneIndex)
{
    s_runtime.ecs = gsk_renderer_active_scene(s_runtime.renderer, sceneIndex);
}
