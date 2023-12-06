/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "collider_setup-system.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/device/device.h"

#include "physics/physics_collision.h"
#include "physics/physics_solver.h"

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_COLLIDER))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCollider *collider   = gsk_ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    collider->isColliding = false; // TODO: remove <- this is for testing

    collider->pCollider = (gsk_Collider *)malloc(sizeof(gsk_Collider));
    ((gsk_Collider *)collider->pCollider)->collider_data_type = collider->type;

    //(gsk_Collider *)(collider->pCollider).position = transform->position;

    // TODO: collider types
    if (collider->type == 1) {
        gsk_SphereCollider *sphereCollider = malloc(sizeof(gsk_SphereCollider));
        sphereCollider->radius             = .20f;
        // glm_vec3_copy(transform->position, sphereCollider->center);
        // sphereCollider.center = transform.position;

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_SphereCollider *)sphereCollider;

    } else if (collider->type == 2) {
        gsk_PlaneCollider *planeCollider = malloc(sizeof(gsk_PlaneCollider));
        // planeCollider->distance      = 0.0175f;
        planeCollider->distance = 10;
        glm_vec3_zero(planeCollider->plane);
        glm_vec3_zero(planeCollider->normal);
        glm_vec3_copy(transform->position, planeCollider->plane);
        LOG_INFO("%f\t%f\t%f",
                 planeCollider->plane[0],
                 planeCollider->plane[1],
                 planeCollider->plane[2]);
        // planeCollider->plane         = transform->position;

        glm_vec3_normalize_to(planeCollider->plane, planeCollider->normal);

        LOG_INFO("%f\t%f\t%f",
                 planeCollider->normal[0],
                 planeCollider->normal[1],
                 planeCollider->normal[2]);

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_PlaneCollider *)planeCollider;
    }
}

static void
update(gsk_Entity e)
{
    // test for collisions
    if (!(gsk_ecs_has(e, C_COLLIDER))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCollider *collider   = gsk_ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    collider->isColliding = 0;

    for (int i = 0; i < e.ecs->nextIndex; i++) {
        if (e.index == (gsk_EntityId)i) continue; // do not check self

        // TODO: fix look-up
        gsk_Entity e_compare = {
          .id    = (gsk_EntityId)i + 1,
          .index = (gsk_EntityId)i,
          .ecs   = e.ecs,
        };

        if (!gsk_ecs_has(e_compare, C_COLLIDER)) continue;
        if (!gsk_ecs_has(e_compare, C_TRANSFORM)) continue;

        struct ComponentCollider *compareCollider =
          gsk_ecs_get(e_compare, C_COLLIDER);

        struct ComponentTransform *compareTransform =
          gsk_ecs_get(e_compare, C_TRANSFORM);

        gsk_CollisionPoints points = {.has_collision = 0};

        //
        // determine which collision-test function to use
        //
        // sphere v. plane
        if (collider->type == 1 && compareCollider->type == 2) {
            points = gsk_physics_collision_find_sphere_plane(
              ((gsk_Collider *)collider->pCollider)->collider_data,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              transform->position,
              compareTransform->position);
        }

        // sphere v. sphere
#if 1
        else if (collider->type == 1 && compareCollider->type == 1) {
            points = gsk_physics_collision_find_sphere_sphere(
              ((gsk_Collider *)collider->pCollider)->collider_data,
              ((gsk_Collider *)compareCollider->pCollider)->collider_data,
              transform->position,
              compareTransform->position);
        }
#endif

        // plane v. sphere
        else if (collider->type == 2 && compareCollider->type == 1) {
            points = gsk_physics_collision_find_plane_sphere(
              collider->pCollider,
              compareCollider->pCollider,
              transform->position,
              compareTransform->position);
        }

        // Collision points
        if (points.has_collision) {

            if (gsk_ecs_has(e, C_RIGIDBODY)) {
                struct ComponentRigidbody *rigidbody =
                  gsk_ecs_get(e, C_RIGIDBODY);

                // Create a new collision result using our points
                // TODO: Send objects A and B
                gsk_CollisionResult result = {
                  .points = points,
                };

                // Send that over to the rigidbody solver list
                gsk_physics_solver_push((gsk_PhysicsSolver *)rigidbody->solver,
                                        result);
            }

            collider->isColliding = points.has_collision;
            break; // TODO: should not break.
                   //- instead, get a collection of points
        }
    }
}

void
s_collider_setup_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init        = (gsk_ECSSubscriber)init,
                              .destroy     = NULL,
                              .render      = NULL,
                              .update      = (gsk_ECSSubscriber)update,
                              .late_update = NULL,
                            }));
}
