#ifndef H_DEMO_SCENES
#define H_DEMO_SCENES

#include <core/graphics/renderer/v1/renderer.h>
#include <ecs/ecs.h>

#ifdef __cplusplus
extern "C" {
#endif

// void demo_scenes_create(int scene, ECS *ecs, Renderer *renderer);
void
demo_scenes_create(ECS *ecs, Renderer *renderer);

#ifdef __cplusplus
}
#endif

#endif // H_DEMO_SCENES