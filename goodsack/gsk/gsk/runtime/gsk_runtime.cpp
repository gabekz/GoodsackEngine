/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
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

#include "gsk_generated/GoodsackEngineConfig.h"

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

// included here for activating core ECS systems
#include "entity/modules/modules_systems.h"

#include "console/console_cmd.h"

#if GSK_USING_COMPOSER
#include "core/audio/music_composer.h"
#include "core/audio/music_composer_loader.hpp"
#endif // GSK_USING_COMPOSER

#include "entity/ecs.h"

#define _TOTAL_ASSET_CACHES 2
#define _TEST_WRITE_PNG     FALSE
#define _HOT_IS_FALLBACK    FALSE

#define _DEFAULT_MAX_FPS 250

extern "C" {
static struct
{
    gsk_ECS *ecs;
    gsk_Renderer *renderer;

    gsk_AssetCache *pp_asset_caches[_TOTAL_ASSET_CACHES];
    u32 cache_cnt;
    char proj_scheme[GSK_FS_MAX_SCHEME];

    gsk_AssetRef *p_default_texture;
    gsk_AssetRef *p_default_audio;
    gsk_AssetRef *p_default_model;
    gsk_AssetRef *p_default_material;
    gsk_AssetRef *p_default_shader;
    gsk_AssetRef *p_default_gcfg;

#if GSK_RUNTIME_USE_DEBUG
    gsk::tools::DebugToolbar *p_debug_toolbar;
    gsk_EntityId selected_entity_id;
#endif // GSK_RUNTIME_DEBUG

    struct
    {
        u8 fs_mode;    // 0 = gpak; 1 = hot
        u8 build_gpak; // 0 = FALSE; 1 = TRUE
        u8 using_lua;  // 0 = FALSE; 1 = TRUE
    } options;

    struct
    {
        char map_uri[GSK_FS_MAX_PATH];
        u8 map_from_runtime;
    } map_setup;

    struct
    {
        u8 is_lua_running;
    } status;

    char bin_directory[256] = "";

    struct
    {
        u32 collision_layer_matrix[ECS_MAX_LAYERS];
    } layers;

    gsk_Path lua_init_path;

} s_runtime;
} // extern "C"

static void
__cmd_echo(u32 argc, const char **argv)
{
    for (u32 i = 0; i < argc; i++)
    {
        LOG_PRINT("%s", argv[i]);
        if (i + 1 < argc) { LOG_PRINT(" "); }
    }
}

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
                strcpy(s_runtime.map_setup.map_uri, value.c_str());
                s_runtime.map_setup.map_from_runtime = 1;
            }
        }
        // Check flags without values
        else
        {
            // errlevel switch
            if (arg == "--errlevel")
            {
                logger_setLevel(LogLevel_ERROR);
            }
            // hot switch
            else if (arg == "--hot")
            {
                LOG_INFO("Set FS Mode to RunHot");
                s_runtime.options.fs_mode = 1;
            }
            // gpak switch
            else if (arg == "--testgpak")
            {
                LOG_INFO("Testing GPAK (overriding FS Mode to RunHot)");
                s_runtime.options.build_gpak = 1;
                s_runtime.options.fs_mode    = 1;
            }
            // lua-disable switch
            else if (arg == "--no-lua")
            {
                LOG_INFO("Lua disabled (--no-lua switch passed)");
                s_runtime.options.using_lua = 0;
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
    /*==== Get binary directory + log path ===========================*/

    strcpy(s_runtime.bin_directory, argv[0]);
    gsk_filesystem_str_to_forward_slash(s_runtime.bin_directory);
    gsk_filesystem_strip_filename(s_runtime.bin_directory);

    char log_path[256] = "";
    sprintf(log_path, "%s/log.txt", s_runtime.bin_directory);

    char gpak_path[256] = "";
    sprintf(gpak_path, "%s/data/", s_runtime.bin_directory);

    /*==== Initialize Logger =========================================*/

    int logStat = logger_initConsoleLogger(NULL);
    // logger_initFileLogger(log_path, 0, 0);

    logger_setLevel(LogLevel_DEBUG);
    logger_setDetail(LogDetail_SIMPLE);

    if (logStat != 0) { LOG_INFO("Initialized Console Logger"); }
#if SYS_DEBUG
    LOG_INFO("Root directory: %s", root_dir);
#endif // SYS_DEBUG

    LOG_INFO("PATH: %s", s_runtime.bin_directory);
    LOG_INFO("LOG_PATH: %s", log_path);

    s_runtime.cache_cnt          = 0; // TODO: Change this.
    s_runtime.options.fs_mode    = 0; // TODO: Change this.
    s_runtime.options.build_gpak = 0; // TODO: Change this.
    s_runtime.options.using_lua  = 1; // TODO: Change this.

    s_runtime.map_setup.map_from_runtime = 0; // TODO: Change this.
    s_runtime.status.is_lua_running      = 0; // TODO: Change this.

    _gsk_check_args(argc, argv);

    switch (gsk_device_getGraphics())
    {
    case GRAPHICS_API_OPENGL: LOG_INFO("Device API is OpenGL"); break;
    case GRAPHICS_API_VULKAN: LOG_INFO("Device API is Vulkan"); break;
    default: LOG_ERROR("Device API Failed to retreive Graphics Backend"); break;
    }

    /*==== Setup gsk filesystem (uri) ================================*/

    char newroot[256] = "";
    sprintf(newroot, "%s/data", s_runtime.bin_directory);

    strcpy(s_runtime.proj_scheme, root_scheme);
    gsk_filesystem_initialize(s_runtime.bin_directory, root_dir, root_scheme);

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

    // GPAK
    if (s_runtime.options.fs_mode == 0)
    {
        for (int i = 0; i < _TOTAL_ASSET_CACHES; i++)
        {
            gsk_gpak_reader_fill_cache(s_runtime.pp_asset_caches[i]);
        }
    }

// HOT
#if !(_HOT_IS_FALLBACK)
    else if (s_runtime.options.fs_mode == 1)
#endif
    {
        // TODO: filesystem traverse should be sorted to be platform-agnostic
        s_runtime.cache_cnt = 0;
        gsk_filesystem_traverse(_GOODSACK_FS_DIR_DATA,
                                _gsk_runtime_cache_asset_file);

        s_runtime.cache_cnt = 1;
        gsk_filesystem_traverse(root_dir, _gsk_runtime_cache_asset_file);
    }

    // set fallback assets
    {
        gsk_AssetCache *p_fallback_cache =
          rt_get_asset_cache_index(GSK_ASSET_FALLBACK_CACHE_INDEX);

        s_runtime.p_default_texture =
          _gsk_asset_get_internal(p_fallback_cache,
                                  "gsk://textures/defaults/missing_1.png",
                                  GSK_ASSET_FETCH_IMPORT);

        s_runtime.p_default_audio = _gsk_asset_get_internal(
          p_fallback_cache, "gsk://audio/boing.wav", GSK_ASSET_FETCH_IMPORT);

        s_runtime.p_default_model = _gsk_asset_get_internal(
          p_fallback_cache, "gsk://models/cube.obj", GSK_ASSET_FETCH_IMPORT);

        s_runtime.p_default_material =
          _gsk_asset_get_internal(p_fallback_cache,
                                  "gsk://fallback/fallback.material",
                                  GSK_ASSET_FETCH_IMPORT);

        s_runtime.p_default_shader =
          _gsk_asset_get_internal(p_fallback_cache,
                                  "gsk://shaders/basic_unlit.shader",
                                  GSK_ASSET_FETCH_IMPORT);

        s_runtime.p_default_gcfg =
          _gsk_asset_get_internal(p_fallback_cache,
                                  "gsk://fallback/fallback.gcfg",
                                  GSK_ASSET_FETCH_IMPORT);
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

    // NOTE: test build_gpak requires hot-loading
    if (s_runtime.options.build_gpak)
    {
        for (int i = 0; i < _TOTAL_ASSET_CACHES; i++)
        {
            gsk_GpakWriter writer =
              gsk_gpak_writer_init(s_runtime.pp_asset_caches[i], gpak_path);

            gsk_gpak_writer_populate_cache(&writer);
            gsk_gpak_writer_close(&writer);
        }

#if GSK_TESTGPAK_EXIT
        exit(0);
#endif
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

    /*==== Initialize Layer Matrix ===================================*/

    for (int i = 0; i < ECS_MAX_LAYERS; i++)
    {
        s_runtime.layers.collision_layer_matrix[i] = 0xFFFFFFFF;
    }

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

    s_runtime.selected_entity_id = 0;

#endif // GSK_RUNTIME_USE_DEBUG

    /*==== Runtime setup =============================================*/

    // FPS Counter
    gsk_device_resetTime();

    // Initialize Graphics Settings
    gsk_device_setGraphicsSettings((gsk_GraphicsSettings {
      .max_fps      = _DEFAULT_MAX_FPS,
      .swapInterval = 1,
    }));
    // Initialize gsk_Input
    gsk_device_setInput((gsk_Input {.cursor_position = {0, 0}}));
    device_setCursorState(INIT_CURSOR_LOCKED, INIT_CURSOR_VISIBLE);

    // Initialize audio interface
    openal_init();

#if USING_RUNTIME_LOADING_SCREEN
    if (GSK_DEVICE_API_OPENGL)
    {
        glfwSwapInterval(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        vec2 text_pos   = {0.0f, 0.0f};
        vec3 text_color = {1.0f, 1.0f, 1.0f};

        gsk_GuiText *loading_text =
          gsk_gui_text_create("Loading", text_pos, text_color);

        const u32 canvas_shader_id =
          s_runtime.renderer->info_canvas.p_material->shaderProgram->id;

        for (int i = 0; i < 2; i++)
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#if RUNTIME_LOADING_TEXT
            gsk_gui_text_draw(loading_text, canvas_shader_id);
#endif // RUNTIME_LOADING_TEXT
            glfwSwapBuffers(s_runtime.renderer->window); // we need to swap.
        }
    }
#endif // RUNTIME_LOADING_SCREEN

    // Intialize TEST commands
    gsk_console_cmd_register("echo", "echo <text..>", __cmd_echo);

    // Initialize Lua

    if (s_runtime.options.using_lua)
    {
        // Main Lua entry
        // TODO: possibly refactor this path (make mutable)
        char path[GSK_FS_MAX_PATH];
        strcpy(path, root_scheme);
        strcat(path, "://scripts/main.lua");
        s_runtime.lua_init_path = gsk_filesystem_uri_to_path(path);

        s_runtime.status.is_lua_running =
          LuaInit(s_runtime.lua_init_path.path, s_runtime.ecs);

        if (s_runtime.status.is_lua_running == false)
        {
            LOG_ERROR("Failed to initialize lua");
        }
    }

    // TODO: move this info_canvas stuff to renderer init

    char str_info[256];
    sprintf(str_info,
            "Goodsack Engine v%d.%d.%d.%d-%s(%s)",
            GOODSACK_VERSION_MAJOR,
            GOODSACK_VERSION_MINOR,
            GOODSACK_VERSION_PATCH,
            GOODSACK_VERSION_TWEAK,
            GOODSACK_VERSION_STATUS,
            _GOODSACK_GIT_HASH_GSK);

    vec2 text_info_pos = {4.0f, 0.0f};
    vec3 text_info_col = {1.0f, 1.0f, 1.0f};

    gsk_GuiText *text_info =
      gsk_gui_text_create(str_info, text_info_pos, text_info_col);
    gsk_gui_canvas_add_text(&s_runtime.renderer->info_canvas, text_info);

#endif
    return 0;
}

void
gsk::runtime::rt_loop()
{
    gsk_renderer_start(
      s_runtime.renderer); // Initialization for the render loop

    // TODO: ECS_INIT

    // TODO: should not be handled in runtime
    if (s_runtime.status.is_lua_running)
    {
        // Register components in Lua ECS
        // TODO: automate (for each component and grab name)

        entity::LuaEventStore::GetInstance().m_ecs = s_runtime.ecs;
        for (int i = 0; i < ECSCOMPONENT_LAST + 1; i++)
        {
            ECSComponentType type = (ECSComponentType)(i);
            entity::LuaEventStore::GetInstance().RegisterComponentList(
              type, gsk_ecs_get_component_name(type));
        }

        // ECS Lua Init
        entity::LuaEventStore::ECSEvent(ECS_INIT); // TODO: REMOVE
    }

#if GSK_USING_COMPOSER
    gsk_MusicComposer composer = gsk_music_composer_create();
    // gsk::audio::composer::create_from_json(GSK_PATH("gsk://composer.json"));
#endif // GSK_USING_COMPOSER

    f64 last_time        = 0.0f;
    f64 accumulated_time = 0.0f;

    // Main Engine Loop
    while (!glfwWindowShouldClose(s_runtime.renderer->window))
    {
        double time_sec = glfwGetTime();

        f64 dt    = time_sec - last_time;
        last_time = time_sec;

        accumulated_time += dt;

        f64 frame_time = 1.0f / (f64)gsk_device_getGraphicsSettings().max_fps;
        if (accumulated_time >= frame_time)
        {
            gsk_device_updateTime(time_sec);
            accumulated_time = 0;

            // LOG_INFO("frame start");

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

                if (s_runtime.status.is_lua_running)
                {
                    entity::LuaEventStore::ECSEvent(ECS_UPDATE);
                }

                gsk_renderer_tick(s_runtime.renderer);

#if GSK_RUNTIME_USE_DEBUG
                s_runtime.p_debug_toolbar->update();
                s_runtime.p_debug_toolbar->render();
#endif // GSK_RUNTIME_USE_DEBUG

                glfwSwapBuffers(s_runtime.renderer->window); // we need to swap.
            }

            // Vulkan
            else if (GSK_DEVICE_API_VULKAN)
            {
                gsk_renderer_tick(s_runtime.renderer);

#if GSK_RUNTIME_USE_DEBUG
                s_runtime.p_debug_toolbar->update();
                s_runtime.p_debug_toolbar->render();
#endif // GSK_RUNTIME_USE_DEBUG

                vulkan_render_draw_end(s_runtime.renderer->vulkanDevice,
                                       s_runtime.renderer->window);
            }
        }
    }

    LOG_INFO("Closing Application");

//
// Cleanup
//

// Delete all ECS Entities
#if 1
    for (int i = 0; i < s_runtime.renderer->sceneC; i++)
    {
        if (s_runtime.renderer->sceneL[i] == NULL) { continue; }

        gsk_ECS *p_ecs = s_runtime.renderer->sceneL[i]->ecs;

        if (p_ecs == NULL) { continue; }

        // mark each entity for deletion
        for (int j = 0; j < p_ecs->nextIndex; j++)
        {
            gsk_ecs_ent_destroy(gsk_ecs_ent(p_ecs, p_ecs->p_ent_ids[j]));
        }

        // call ECS_DESTROY for each ECS handler
        gsk_ecs_event(s_runtime.renderer->sceneL[i]->ecs, ECS_DESTROY);
    }
#endif

    if (GSK_DEVICE_API_VULKAN)
    {
        vkDeviceWaitIdle(s_runtime.renderer->vulkanDevice->device);
        vulkan_device_cleanup(s_runtime.renderer->vulkanDevice);
    }

#if GSK_RUNTIME_USE_DEBUG
    delete (s_runtime.p_debug_toolbar);
#endif // GSK_RUNTIME_USE_DEBUG

    // cleanup Lua handler
    if (s_runtime.status.is_lua_running) { entity::LuaEventStore::Cleanup(); }

    // TODO: asset_cache cleanup

    // cleanup audio driver
    openal_cleanup();

    glfwTerminate();
}

void
gsk::runtime::rt_activate_ecs_systems(gsk_ECS *p_ecs)
{
    // Activate ECS Systems

    LOG_DEBUG("Activating built-in ECS Systems");

    s_transform_init(p_ecs);

    s_camera_init(p_ecs);
    s_model_draw_init(p_ecs);
    s_audio_listener_init(p_ecs);
    s_audio_source_init(p_ecs);
    s_animator_init(p_ecs);

    // Physics Systems
    // order is important here..
    s_collider_setup_system_init(p_ecs);      // initialize colliders
    s_rigidbody_system_init(p_ecs);           // rigidbody initialization
    s_collision_detection_system_init(p_ecs); // check for collisions
    s_physics_world_system_init(p_ecs);       // physics simulation (singelton)

    // Player Controller
    s_player_controller_system_init(p_ecs);

    // Misc Systems
    s_health_setup_init(p_ecs);
    s_particles_ecs_system_init(p_ecs);

    // Light System
    s_light_setup_system_init(p_ecs);

    s_collider_debug_draw_system_init(p_ecs);
}

void
gsk::runtime::rt_set_scene(u16 scene_index)
{
    s_runtime.ecs = gsk_renderer_active_scene(s_runtime.renderer, scene_index);

    if (s_runtime.ecs->systems_size <= 0)
    {
        gsk::runtime::rt_activate_ecs_systems(s_runtime.ecs);
    }
}

void
gsk::runtime::rt_set_layer_mask(u32 layer_index, u32 layer_mask)
{
    s_runtime.layers.collision_layer_matrix[layer_index] = layer_mask;
}

u8
gsk::runtime::rt_check_layer_mask(u32 layer_a, u32 layer_b)
{
    u32 *p_layer_mask = s_runtime.layers.collision_layer_matrix;

    u32 lower  = layer_a;
    u32 higher = layer_b;

    if (layer_a > layer_b)
    {
        lower  = layer_b;
        higher = layer_a;
    }

    return (((u32)0x01 << (higher - lower)) & p_layer_mask[lower]) > 0;
}

gsk_ECS *
gsk::runtime::rt_get_ecs()
{
    return s_runtime.renderer->sceneL[s_runtime.renderer->activeScene]->ecs;
}

gsk_Renderer *
gsk::runtime::rt_get_renderer()
{
    return s_runtime.renderer;
}

gsk_AssetCache *
gsk::runtime::rt_get_asset_cache_index(u32 index)
{
    if (index > _TOTAL_ASSET_CACHES)
    {
        LOG_ERROR("cannot get asset cache which does not exist");
        return NULL;
    }

    return s_runtime.pp_asset_caches[index];
}

gsk_AssetCache *
gsk::runtime::rt_get_asset_cache(const char *uri_str)
{
    // validate uri
    gsk_URI uri  = gsk_filesystem_uri(uri_str);
    void *p_data = NULL;

    if (!strcmp(uri.scheme, GSK_FS_GSK_SCHEME))
    {
        return rt_get_asset_cache_index(0);
    } else if (!strcmp(uri.scheme, s_runtime.proj_scheme))
    {

        return rt_get_asset_cache_index(1);
    }
    LOG_ERROR("Failed to find asset cache for: %s", uri_str);
    return NULL;
}

void *
gsk::runtime::rt_get_debug_toolbar()
{
#if SYS_DEBUG
    return s_runtime.p_debug_toolbar;
#else
    return NULL;
#endif // SYS_DEBUG
}

gsk_AssetRef *
gsk::runtime::rt_get_fallback_asset(GskAssetType type)
{
    gsk_AssetRef *p_ret = NULL;

    switch (type)
    {
    case GskAssetType_Texture: p_ret = s_runtime.p_default_texture; break;
    case GskAssetType_Audio: p_ret = s_runtime.p_default_audio; break;
    case GskAssetType_Model: p_ret = s_runtime.p_default_model; break;
    case GskAssetType_Material: p_ret = s_runtime.p_default_material; break;
    case GskAssetType_Shader: p_ret = s_runtime.p_default_shader; break;
    case GskAssetType_GCFG: p_ret = s_runtime.p_default_gcfg; break;
    default: p_ret = NULL; break;
    }

    return p_ret;
}

char *
gsk::runtime::rt_get_startup_map()
{
    if (s_runtime.map_setup.map_from_runtime == 0) { return NULL; }
    return s_runtime.map_setup.map_uri;
}

gsk_EntityId
gsk::runtime::rt_get_hovered_entity_id()
{
#if SYS_DEBUG
    if (s_runtime.renderer->hovered_entity_index == 0 ||
        s_runtime.p_debug_toolbar->is_focused())
    {
        return 0;
    }

    u32 index = s_runtime.renderer->hovered_entity_index - 1;

    gsk_ECS *p_ecs  = gsk::runtime::rt_get_ecs();
    gsk_EntityId id = p_ecs->p_ent_ids[index];

    return id;
#else
    return 0;
#endif // SYS_DEBUG
}

gsk_EntityId
gsk::runtime::rt_get_debug_entity_id()
{
#if SYS_DEBUG
    return s_runtime.selected_entity_id;
#else
    return (gsk_EntityId)0;
#endif // SYS_DEBUG
}

void
gsk::runtime::rt_set_debug_entity_id(gsk_EntityId entity_id)
{
#if SYS_DEBUG
    if (entity_id >= ECS_ID_FIRST)
    {
        s_runtime.selected_entity_id = entity_id;
        LOG_TRACE("set RT debug entity_id to: %d",
                  s_runtime.selected_entity_id);
    }
#endif // SYS_DEBUG
}

void *
gsk::runtime::rt_get_lua_state()
{
    if (!s_runtime.status.is_lua_running) { return NULL; }

    return entity::LuaEventStore::getLuaState();
}

void
gsk::runtime::rt_lua_reload()
{
    if (!s_runtime.status.is_lua_running) { return; }

    s_runtime.status.is_lua_running = LuaClose();

    s_runtime.status.is_lua_running =
      LuaInit(s_runtime.lua_init_path.path, s_runtime.ecs);

    if (s_runtime.status.is_lua_running == false)
    {
        LOG_ERROR("Failed to initialize lua");
    }
}