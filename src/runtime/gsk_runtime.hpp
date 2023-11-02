#ifndef HPP_GSK_RUNTIME
#define HPP_GSK_RUNTIME

#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <core/graphics/renderer/v1/renderer.h>
#include <entity/v1/ecs.h>

#ifdef __cplusplus
}
#endif

// #define RENDERER_2
#define USING_LUA                    1
#define USING_RUNTIME_LOADING_SCREEN 1
#define USING_JOYSTICK_CONTROLLER    1

// Starting cursor state
#define INIT_CURSOR_LOCKED  1
#define INIT_CURSOR_VISIBLE 0

#define GSK_RUNTIME_USE_DEBUG 1

ui32
gsk_runtime_setup(const char *project_root, int argc, char *argv[]);

void
gsk_runtime_loop();

ECS *
gsk_runtime_get_ecs();
Renderer *
gsk_runtime_get_renderer();

void
gsk_runtime_set_scene(ui16 sceneIndex);

#endif // H_GSK_RUNTIME