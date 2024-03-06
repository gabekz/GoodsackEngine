/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "mod_physics.h"

#include "util/sysdefs.h"

#include "entity/ecs.h"
#include "physics/physics_collision.h"
#include "physics/physics_types.h"

// todo: gsk_ECSCaller (includes entity info)

gsk_mod_RaycastResult
gsk_mod_physics_raycast(gsk_Entity entity_caller, gsk_Raycast *raycast)
{
    for (int i = 0; i < entity_caller.ecs->nextIndex; i++) {
        if (entity_caller.index == (gsk_EntityId)i)
            continue; // do not check self

        // TODO: fix look-up
        gsk_Entity e_compare = {
          .id    = (gsk_EntityId)i + 1,
          .index = (gsk_EntityId)i,
          .ecs   = entity_caller.ecs,
        };

        if (!gsk_ecs_has(e_compare, C_COLLIDER)) continue;
        if (!gsk_ecs_has(e_compare, C_TRANSFORM)) continue;

        struct ComponentCollider *compareCollider =
          gsk_ecs_get(e_compare, C_COLLIDER);

        struct ComponentTransform *compareTransform =
          gsk_ecs_get(e_compare, C_TRANSFORM);

        if (compareCollider->type == COLLIDER_SPHERE) {
            gsk_CollisionPoints points = gsk_physics_collision_find_ray_sphere(
              raycast,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              compareTransform->position);

            return (gsk_mod_RaycastResult) {
              .entity        = e_compare,
              .has_collision = points.has_collision,
            };
        }
    };

    return (gsk_mod_RaycastResult) {
      .entity        = NULL,
      .has_collision = FALSE,
    };
}