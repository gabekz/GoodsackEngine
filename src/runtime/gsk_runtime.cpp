#include "gsk_runtime.hpp"

#include <util/logger.h>
#include <util/sysdefs.h>

#include <core/device/device.h>

#include <core/device/device.h>
#include <core/graphics/lighting/lighting.h>
#include <entity/v1/ecs.h>
#include <tools/debugui.hpp>
#include <wrapper/lua/lua_init.hpp>

#include <entity/lua/eventstore.hpp>

// #define RENDERER_2

#ifdef RENDERER_2
#include <core/graphics/renderer/renderer.hpp>
#else
extern "C" {
#include <core/graphics/renderer/v1/renderer.h>
}
#endif

extern "C" {
static struct
{
    ECS *ecs;
    Renderer *renderer;
    DebugGui *debugGui;

} s_runtime;
}

static void
_gsk_check_args(int argc, char *argv[])
{
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
}

ui32
gsk_runtime_setup(int argc, char *argv[])
{
    // Setup logger
    int logStat = logger_initConsoleLogger(NULL);
    logger_initFileLogger("logs/logs.txt", 0, 0);

    logger_setLevel(LogLevel_TRACE);
    logger_setDetail(LogDetail_SIMPLE);

    if (logStat != 0) { LOG_INFO("Initialized Console Logger"); }

    _gsk_check_args(argc, argv);

    switch (device_getGraphics()) {
    case GRAPHICS_API_OPENGL: LOG_INFO("Device API is OpenGL"); break;
    case GRAPHICS_API_VULKAN: LOG_INFO("Device API is Vulkan"); break;
    default: LOG_ERROR("Device API Failed to retreive Graphics Backend"); break;
    }

    // Initialize Renderer
#ifdef RENDERER_2
    Renderer renderer = new Renderer();
    // ECSManager ecs = Renderer.
    Scene scene0 = renderer->SetActiveScene(0);
#else
    s_runtime.renderer = renderer_init();

    int winWidth  = s_runtime.renderer->windowWidth;
    int winHeight = s_runtime.renderer->windowHeight;

    // Initialize ECS
    s_runtime.ecs = renderer_active_scene(s_runtime.renderer, 0);

    // Lighting information
    vec3 lightPos   = {1.0f, 2.8f, -0.2f};
    vec4 lightColor = {1.0f, 1.0f, 1.0f, 1.0f};

    // UBO Lighting
    s_runtime.renderer->light =
      lighting_initialize((float *)lightPos, (float *)lightColor);

    // Create DebugGui
    s_runtime.debugGui = new DebugGui(s_runtime.renderer);

    // FPS Counter
    device_resetAnalytics();

    device_setGraphicsSettings((GraphicsSettings {.swapInterval = 1}));
    device_setInput(
      (Input {.cursor_position = {0, 0}, .holding_right_button = 0}));

    // Main Lua entry
    LuaInit("../demo/demo_hot/Resources/scripts/main.lua", s_runtime.ecs);

#endif
    return 0;
}

void
gsk_runtime_loop()
{
    renderer_start(s_runtime.renderer); // Initialization for the render loop
    entity::LuaEventStore::GetInstance().m_ecs = s_runtime.ecs;
    entity::LuaEventStore::ECSEvent(ECS_INIT); // TODO: REMOVE
    while (!glfwWindowShouldClose(s_runtime.renderer->window)) {

        device_updateAnalytics(glfwGetTime());
        // LOG_INFO("FPS: %f", device_getAnalytics().currentFps);

        if (DEVICE_API_OPENGL) {
            entity::LuaEventStore::ECSEvent(ECS_UPDATE);
            renderer_tick(s_runtime.renderer);
            s_runtime.debugGui->Render();
            glfwSwapBuffers(s_runtime.renderer->window); // we need to swap.
        } else if (DEVICE_API_VULKAN) {
            glfwPollEvents();
            // debugGui->Update();
            ecs_event(s_runtime.ecs, ECS_UPDATE); // TODO: REMOVE
            vulkan_render_draw_begin(s_runtime.renderer->vulkanDevice,
                                     s_runtime.renderer->window);
            s_runtime.renderer->currentPass = REGULAR;
            ecs_event(s_runtime.ecs, ECS_RENDER);
            s_runtime.debugGui->Render();
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

    delete (s_runtime.debugGui);
    glfwTerminate();
}

ECS *
gsk_runtime_get_ecs()
{
    return s_runtime.ecs;
}
Renderer *
gsk_runtime_get_renderer()
{
    return s_runtime.renderer;
}

void
gsk_runtime_set_scene(ui16 sceneIndex)
{
    s_runtime.ecs = renderer_active_scene(s_runtime.renderer, sceneIndex);
}