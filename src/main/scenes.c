#include "scenes.h"

#include <ecs/ecs.h>
#include <core/renderer/v1/renderer.h>

#include <components/components.h>


void build_scene0(ECS *ecs, Renderer *renderer) {
	
	
	ecs = renderer_active_scene(renderer, 2);

    Entity camera2 = ecs_new(ecs);
    _ecs_add_internal(camera2,
            C_CAMERA,
            (void *)(&(struct ComponentCamera){
              .position = {0.0f, 0.0f, 2.0f},
              .axisUp   = {0.0f, 1.0f, 0.0f},
              .speed    = 0.05f,
            }));
}