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

ui32
gsk_runtime_setup(int argc, char *argv[]);

void
gsk_runtime_loop();

ECS *
gsk_runtime_get_ecs();
Renderer *
gsk_runtime_get_renderer();

void
gsk_runtime_set_scene(ui16 sceneIndex);

#endif // H_GSK_RUNTIME