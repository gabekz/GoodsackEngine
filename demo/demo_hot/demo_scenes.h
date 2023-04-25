#ifndef H_DEMO_SCENES
#define H_DEMO_SCENES

#include <core/graphics/renderer/v1/renderer.h>
#include <ecs/ecs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOAD_ALL_SCENES 0
#define INITIAL_SCENE  5 

void
demo_scenes_create(ECS *ecs, Renderer *renderer);

#ifdef __cplusplus
}
#endif

#endif // H_DEMO_SCENES