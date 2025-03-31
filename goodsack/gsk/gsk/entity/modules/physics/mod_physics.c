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
          .id    = entity_caller.ecs->p_ent_ids[i],
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
        } else if (compareCollider->type == COLLIDER_CAPSULE)
        {
            points = gsk_physics_collision_find_ray_capsule(
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

gsk_mod_RaycastResult
gsk_mod_physics_capsuletest(gsk_Entity entity_caller,
                            vec3 origin,
                            vec3 direction,
                            float max_distance)
{

    f32 closest_range         = 6000;
    u32 closest_id            = 0;
    gsk_mod_RaycastResult ret = {0};

    // get all points
    vec3 ray_vec;
    // glm_vec3_normalize_to(direction, ray_vec);
    glm_vec3_scale(direction, 0.025f, ray_vec);
    glm_vec3_add(origin, ray_vec, ray_vec);

    vec3 base  = {0.0f, 1.255f, 0.0f};
    vec3 tip   = {0.0f, 0.5f, 0.0f};
    f32 radius = 0.22f;

    gsk_CapsuleCollider capsuleCollider = {.radius = radius};
    glm_vec3_copy(base, capsuleCollider.base);
    glm_vec3_copy(tip, capsuleCollider.tip);

    // gsk_CollisionPoints points_ret;
    // points_ret.has_collision = FALSE;

    for (int i = 0; i < entity_caller.ecs->nextIndex; i++)
    {
        if (entity_caller.index == (gsk_EntityId)i)
            continue; // do not check self

        // TODO: fix look-up
        gsk_Entity e_compare = {
          .id    = entity_caller.ecs->p_ent_ids[i],
          .index = (gsk_EntityId)i,
          .ecs   = entity_caller.ecs,
        };

        // if (e_compare.id == 312) continue; // do not check self

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
            points = gsk_physics_collision_find_capsule_sphere(
              &capsuleCollider,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              ray_vec,
              compareTransform->position);

        } else if (compareCollider->type == COLLIDER_BOX)
        {
            points = gsk_physics_collision_find_capsule_box(
              &capsuleCollider,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              ray_vec,
              compareTransform->position);
        } else if (compareCollider->type == COLLIDER_PLANE)
        {
            points = gsk_physics_collision_find_capsule_plane(
              &capsuleCollider,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              ray_vec,
              compareTransform->position);
        }

        if (points.has_collision && points.point_a[1] > 0.1f)
        {
            f32 ray_range = glm_vec3_distance(origin, points.point_b);
            if (ray_range < closest_range && max_distance <= closest_range)
            {
                ret = (gsk_mod_RaycastResult) {
                  .entity        = e_compare,
                  .hit_position  = {points.point_b[0],
                                   points.point_b[1],
                                   points.point_b[2]},
                  .hit_normal    = {points.normal[0],
                                 points.normal[1],
                                 points.normal[2]},
                  .has_collision = TRUE,
                };

                closest_range = ray_range;
                closest_id    = e_compare.id;
            }
        }
    }

    return ret;
}