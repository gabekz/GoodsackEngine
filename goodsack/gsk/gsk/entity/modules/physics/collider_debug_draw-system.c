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
#include "tools/debug/debug_draw_bounds.h"

static void
render(gsk_Entity entity)
{
    if (entity.ecs->renderer->currentPass != GskRenderPass_Skybox) { return; }

    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;
    if (!(gsk_ecs_has(entity, C_TRANSFORM))) return;

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

        gsk_debug_draw_bounds(entity.ecs->renderer->debugContext,
                              p_box->bounds,
                              transform->model,
                              (collider->isColliding) ? VCOL_BLUE : VCOL_GREEN);
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