/*
 * Copyright (c) 2025-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "collider_debug_draw-system.h"

#include "util/maths.h"
#include "util/sysdefs.h"
#include "util/vec_colors.h"

#include "entity/ecs.h"
#include "physics/physics_types.h"
#include "runtime/gsk_runtime_wrapper.h"
#include "tools/debug/debug_draw_bounds.h"

static void
_get_collider_color(u8 is_colliding, u8 is_trigger, float *out_col)
{
    if (is_colliding)
    {
        if (is_trigger)
        {
            glm_vec4_copy(VCOL_PURPLE, out_col);
            return;
        }
        glm_vec4_copy(VCOL_BLUE, out_col);
        return;
    }

    if (is_trigger)
    {
        glm_vec4_copy(VCOL_ORANGE, out_col);
        return;
    }
    glm_vec4_copy(VCOL_GREEN, out_col);
}

static void
render(gsk_Entity entity)
{
    if (entity.ecs->renderer->currentPass != GskRenderPass_Skybox) { return; }

    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;
    if (!(gsk_ecs_has(entity, C_TRANSFORM))) return;

    const gsk_DebugPhysicsOptions *physics_options =
      &entity.ecs->renderer->debugContext->physics_options;

    if (physics_options->draw_collisions == FALSE) { return; }

    // TODO: loop through children to render them as well
    if (physics_options->selected_entity_only == TRUE &&
        entity.id != gsk_runtime_get_debug_entity_id())
    {
        return;
    }

    struct ComponentCollider *collider   = gsk_ecs_get(entity, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(entity, C_TRANSFORM);

    gsk_Collider *p_col = ((gsk_Collider *)collider->pCollider)->collider_data;
    // if (collider->isColliding == FALSE) { return; }

    if (collider->type == COLLIDER_BOX)
    {
        gsk_BoxCollider *p_box = (gsk_BoxCollider *)p_col;

#if 0
        vec4 bounds_color = {
          0.0f, 1.0f, 0.0f, (collider->isColliding) ? 1.0f : 0.5f};
#endif

        vec4 collider_color = GLM_VEC4_ZERO_INIT;
        _get_collider_color(
          collider->isColliding, collider->is_trigger, collider_color);

        gsk_debug_draw_bounds(entity.ecs->renderer->debugContext,
                              p_box->bounds,
                              transform->model,
                              collider_color);
    }
}

void
s_collider_debug_draw_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .render = (gsk_ECSSubscriber)render,
                            }));
}