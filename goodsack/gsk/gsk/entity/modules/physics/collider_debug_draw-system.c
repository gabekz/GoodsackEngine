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

    if (!(gsk_ecs_has(entity, C_COLLIDER))) { return; }
    if (!(gsk_ecs_has(entity, C_TRANSFORM))) { return; }

    const gsk_DebugContext *p_debug_context =
      entity.ecs->renderer->debugContext;

    const gsk_DebugPhysicsOptions *p_physics_options =
      &(p_debug_context->physics_options);

    if (p_debug_context->is_active == FALSE ||
        p_physics_options->draw_collisions == FALSE)
    {
        return;
    }

    // TODO: loop through children to render them as well
    if (p_physics_options->selected_entity_only == TRUE &&
        entity.id != gsk_runtime_get_debug_entity_id())
    {
        return;
    }

    gsk_C_Collider *collider   = gsk_ecs_get(entity, C_COLLIDER);
    gsk_C_Transform *transform = gsk_ecs_get(entity, C_TRANSFORM);

    gsk_Collider *p_col = ((gsk_Collider *)collider->pCollider)->collider_data;

    if (collider->type == COLLIDER_BOX)
    {
        gsk_BoxCollider *p_box = (gsk_BoxCollider *)p_col;

        vec4 collider_color = GLM_VEC4_ZERO_INIT;
        _get_collider_color(
          collider->isColliding, collider->is_trigger, collider_color);

        mat4 matrix = GLM_MAT4_IDENTITY_INIT;
        glm_translate(matrix, transform->position);
        glm_mat4_mul(matrix, transform->m4_rotation, matrix);

        gsk_debug_draw_bounds(entity.ecs->renderer->debugContext,
                              p_box->bounds,
                              matrix,
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