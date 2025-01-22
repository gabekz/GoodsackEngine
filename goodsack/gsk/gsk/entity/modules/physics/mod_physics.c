/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "mod_physics.h"

#include "util/sysdefs.h"

#include "entity/ecs.h"
#include "physics/physics_collision.h"
#include "physics/physics_types.h"

// todo: gsk_ECSCaller (includes entity info)

gsk_mod_RaycastResult
gsk_mod_physics_raycast(gsk_Entity entity_caller,
                        gsk_Raycast *raycast,
                        float range)
{
    f32 closest_range         = 6000;
    gsk_mod_RaycastResult ret = {.entity = NULL, .has_collision = FALSE};

    for (int i = 0; i < entity_caller.ecs->nextIndex; i++)
    {
        if (entity_caller.index == (gsk_EntityId)i)
            continue; // do not check self

        // TODO: fix look-up
        gsk_Entity e_compare = {
          .id    = entity_caller.ecs->ids[i],
          .index = (gsk_EntityId)i,
          .ecs   = entity_caller.ecs,
        };

        if (!gsk_ecs_has(e_compare, C_COLLIDER)) continue;
        if (!gsk_ecs_has(e_compare, C_TRANSFORM)) continue;

        struct ComponentCollider *compareCollider =
          gsk_ecs_get(e_compare, C_COLLIDER);

        struct ComponentTransform *compareTransform =
          gsk_ecs_get(e_compare, C_TRANSFORM);

        gsk_CollisionPoints points;
        points.has_collision = FALSE;

        if (compareCollider->type == COLLIDER_SPHERE)
        {
            points = gsk_physics_collision_find_ray_sphere(
              raycast,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              compareTransform->position);

        } else if (compareCollider->type == COLLIDER_BOX)
        {
            points = gsk_physics_collision_find_ray_box(
              raycast,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              compareTransform->position);
        } else if (compareCollider->type == COLLIDER_PLANE)
        {
            points = gsk_physics_collision_find_ray_plane(
              raycast,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              compareTransform->position);
        }

        if (points.has_collision)
        {
            f32 ray_range = glm_vec3_distance(raycast->origin, points.point_a);

            if (range && ray_range <= range && ray_range <= closest_range)
            {
                closest_range = ray_range;

                ret = (gsk_mod_RaycastResult) {
                  .entity        = e_compare,
                  .hit_position  = {points.point_a[0],
                                   points.point_a[1],
                                   points.point_a[2]},
                  .hit_normal    = {points.normal[0],
                                 points.normal[1],
                                 points.normal[2]},
                  .has_collision = TRUE,
                };
            }
        }
    };

    return ret;
}