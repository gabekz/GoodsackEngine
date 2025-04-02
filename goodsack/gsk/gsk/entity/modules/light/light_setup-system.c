#include "light_setup-system.h"

#include "core/graphics/lighting/lighting.h"

#include "util/logger.h"
#include "util/sysdefs.h"

static void
init(gsk_Entity entity)
{
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }
    if (!gsk_ecs_has(entity, C_LIGHT)) { return; }

    struct ComponentTransform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);
    struct ComponentLight *cmp_light         = gsk_ecs_get(entity, C_LIGHT);

    // grab the current renderer and scene
    gsk_Renderer *p_renderer = entity.ecs->renderer;
    gsk_Scene *p_scene       = p_renderer->sceneL[p_renderer->activeScene];

    u32 light_index =
      gsk_lighting_add_light(&p_scene->lighting_data,
                             (f32 *)cmp_transform->world_position,
                             (f32 *)cmp_light->color);

    cmp_light->light_index = light_index;
}

static void
update(gsk_Entity entity)
{
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }
    if (!gsk_ecs_has(entity, C_LIGHT)) { return; }

    struct ComponentTransform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);
    struct ComponentLight *cmp_light         = gsk_ecs_get(entity, C_LIGHT);

    // grab the current renderer and scene
    gsk_Renderer *p_renderer = entity.ecs->renderer;
    gsk_Scene *p_scene       = p_renderer->sceneL[p_renderer->activeScene];

    gsk_LightingData *p_lighting_data = &p_scene->lighting_data;

    if (cmp_light->intensity < 0) { cmp_light->intensity = 0; }

    glm_vec3_copy(cmp_transform->world_position,
                  p_lighting_data->lights[cmp_light->light_index].position);

    vec4 new_color;
    glm_vec4_normalize_to(cmp_light->color, new_color);
    glm_vec4_scale(new_color, cmp_light->intensity, new_color);

    glm_vec4_copy(new_color,
                  p_lighting_data->lights[cmp_light->light_index].color);
}

void
s_light_setup_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init   = (gsk_ECSSubscriber)init,
                              .update = (gsk_ECSSubscriber)update,
                            }));
}