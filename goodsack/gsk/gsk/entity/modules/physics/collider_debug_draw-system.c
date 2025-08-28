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
#include "tools/debug/debug_draw_line.h"
#include "tools/debug/debug_draw_sphere.h"

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

    vec4 collider_color = GLM_VEC4_ZERO_INIT;
    _get_collider_color(
      collider->isColliding, collider->is_trigger, collider_color);

    mat4 matrix = GLM_MAT4_IDENTITY_INIT;
    glm_translate(matrix, transform->world_position);

    // COLLIDER_BOX
    if (collider->type == COLLIDER_BOX)
    {
        glm_mat4_mul(matrix, transform->m4_rotation, matrix);

        gsk_BoxCollider *p_box = (gsk_BoxCollider *)p_col;

        gsk_debug_draw_bounds(entity.ecs->renderer->debugContext,
                              p_box->bounds,
                              matrix,
                              collider_color);
    }
    // COLLIDER_SPHERE
    else if (collider->type == COLLIDER_SPHERE)
    {
        gsk_SphereCollider *p_shere = (gsk_SphereCollider *)p_col;

        gsk_debug_draw_sphere(entity.ecs->renderer->debugContext,
                              p_shere->radius,
                              matrix,
                              collider_color);
    }
    // COLLIDER_CAPSULE
    else if (collider->type == COLLIDER_CAPSULE)
    {
        gsk_CapsuleCollider *p_capsule = (gsk_CapsuleCollider *)p_col;

        mat4 matrix_tip  = GLM_MAT4_IDENTITY_INIT;
        mat4 matrix_base = GLM_MAT4_IDENTITY_INIT;

        vec3 pos_tip  = GLM_VEC3_ZERO_INIT;
        vec3 pos_base = GLM_VEC3_ZERO_INIT;

        glm_vec3_add(transform->position, p_capsule->tip, pos_tip);
        glm_vec3_sub(transform->position, p_capsule->base, pos_base);

        glm_translate(matrix_tip, pos_tip);
        glm_translate(matrix_base, pos_base);

        gsk_debug_draw_sphere(entity.ecs->renderer->debugContext,
                              p_capsule->radius,
                              matrix_tip,
                              collider_color);

        gsk_debug_draw_sphere(entity.ecs->renderer->debugContext,
                              p_capsule->radius,
                              matrix_base,
                              collider_color);

        // lines

        f32 offset = p_capsule->radius;

        vec3 line_offsets[4] = {
          {offset, 0, 0}, {-offset, 0, 0}, {0, 0, offset}, {0, 0, -offset}};

        for (int i = 0; i < 4; i++)
        {
            vec3 start, end;
            glm_vec3_add(pos_base, line_offsets[i], start);
            glm_vec3_add(pos_tip, line_offsets[i], end);

            gsk_debug_draw_line(
              entity.ecs->renderer->debugContext, start, end, collider_color);
        }
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