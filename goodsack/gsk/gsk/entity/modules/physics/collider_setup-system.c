/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "collider_setup-system.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/device/device.h"
#include "core/graphics/mesh/mesh.h"

#include "physics/physics_collision.h"
#include "physics/physics_solver.h"

#define MAX_COLLISION_POINTS 32

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
    if (collider->type == COLLIDER_SPHERE) {
        gsk_SphereCollider *sphereCollider = malloc(sizeof(gsk_SphereCollider));
        sphereCollider->radius             = 1.0f;
        // glm_vec3_copy(transform->position, sphereCollider->center);
        // sphereCollider.center = transform.position;

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_SphereCollider *)sphereCollider;

    } else if (collider->type == COLLIDER_PLANE) {
        gsk_PlaneCollider *planeCollider = malloc(sizeof(gsk_PlaneCollider));

        // planeCollider->distance      = 0.0175f;
        planeCollider->distance = 10;
        glm_vec3_zero(planeCollider->plane);

        // TODO: Calculate normal from plane mesh with orientation
        vec3 planenorm = {0.0f, 1.0f, 0.0f};
        glm_vec3_copy(planenorm, planeCollider->normal);

        glm_vec3_copy(transform->position, planeCollider->plane);

        LOG_INFO("%f\t%f\t%f",
                 planeCollider->plane[0],
                 planeCollider->plane[1],
                 planeCollider->plane[2]);
        // planeCollider->plane         = transform->position;

        // glm_vec3_normalize_to(planeCollider->plane, planeCollider->normal);

        LOG_INFO("%f\t%f\t%f",
                 planeCollider->normal[0],
                 planeCollider->normal[1],
                 planeCollider->normal[2]);

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_PlaneCollider *)planeCollider;
    }

    else if (collider->type == COLLIDER_BOX) {
        gsk_BoxCollider *box_collider = malloc(sizeof(gsk_BoxCollider));
        // glm_vec3_zero(box_collider->bounds[0]);
        // glm_vec3_zero(box_collider->bounds[1]);

        if (gsk_ecs_has(e, C_MODEL)) {
            struct ComponentModel *cmp_model = gsk_ecs_get(e, C_MODEL);
            gsk_MeshData *meshdata = ((gsk_Mesh *)cmp_model->mesh)->meshData;
            glm_vec3_copy(meshdata->boundingBox[0], box_collider->bounds[0]);
            glm_vec3_copy(meshdata->boundingBox[1], box_collider->bounds[1]);
#if 0
            LOG_INFO("Box collider min-bounds: %f\t%f\t%f",
                     box_collider->bounds[0][0],
                     box_collider->bounds[0][1],
                     box_collider->bounds[0][2]);
            LOG_INFO("Box collider max-bounds: %f\t%f\t%f",
                     box_collider->bounds[1][0],
                     box_collider->bounds[1][1],
                     box_collider->bounds[1][2]);
#endif
        }

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_BoxCollider *)box_collider;
    }
}

static void
fixed_update(gsk_Entity e)
{
    // test for collisions
    if (!(gsk_ecs_has(e, C_COLLIDER))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCollider *collider   = gsk_ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    collider->isColliding = 0;

    gsk_CollisionPoints points_list[MAX_COLLISION_POINTS];
    u32 points_list_next = 0;

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

//-----------------------
// parameters used for all collision-check functions
//
// NOTE: this may be a little hacky, but it makes the collision-comparison
// a lot more readable.
#define __CLSN_PARAMS__                                            \
    ((gsk_Collider *)collider->pCollider)->collider_data,          \
      ((gsk_Collider *)compareCollider->pCollider)->collider_data, \
      transform->position, compareTransform->position

        //
        // determine which collision-test function to use
        //
        gsk_CollisionPoints points = {.has_collision = 0};

        if (collider->type == COLLIDER_SPHERE) { // --- Sphere Collider
            switch (compareCollider->type) {
            case COLLIDER_SPHERE:
                points =
                  gsk_physics_collision_find_sphere_sphere(__CLSN_PARAMS__);
                break;
            case COLLIDER_PLANE:
                points =
                  gsk_physics_collision_find_sphere_plane(__CLSN_PARAMS__);
                break;
            case COLLIDER_BOX:
            default: break;
            };
        } else if (collider->type == COLLIDER_PLANE) { // --- Plane Collider
            switch (compareCollider->type) {
            case COLLIDER_SPHERE:
                points =
                  gsk_physics_collision_find_plane_sphere(__CLSN_PARAMS__);
                break;
            case COLLIDER_PLANE:
            case COLLIDER_BOX:
            default: break;
            };
        } else if (collider->type == COLLIDER_BOX) { // --- Box Collider
            switch (compareCollider->type) {
            case COLLIDER_SPHERE:
                points = gsk_physics_collision_find_box_sphere(__CLSN_PARAMS__);
                break;
            case COLLIDER_PLANE:
                points = gsk_physics_collision_find_box_plane(__CLSN_PARAMS__);
                break;
            case COLLIDER_BOX:
            default: break;
            };
        }

        // Collision points
        if (points.has_collision) {

            points_list[points_list_next] = points;
            if (points_list_next >= MAX_COLLISION_POINTS) {
                LOG_CRITICAL("Max collision points exceeded");
            }
            points_list_next++;
        }
    }

    for (u32 i = 0; i < points_list_next; i++) {
        if (points_list[i].has_collision) {
            if (gsk_ecs_has(e, C_RIGIDBODY)) {
                struct ComponentRigidbody *rigidbody =
                  gsk_ecs_get(e, C_RIGIDBODY);

                // Create a new collision result using our points
                // TODO: Send objects A and B
                gsk_CollisionResult result = {
                  .points = points_list[i],
                };

                // Send that over to the rigidbody solver list
                gsk_physics_solver_push((gsk_PhysicsSolver *)rigidbody->solver,
                                        result);
            }

            collider->isColliding = points_list[i].has_collision;
        }
    }
}

void
s_collider_setup_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init         = (gsk_ECSSubscriber)init,
                              .destroy      = NULL,
                              .render       = NULL,
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                              .update       = NULL,
                              .late_update  = NULL,
                            }));
}
