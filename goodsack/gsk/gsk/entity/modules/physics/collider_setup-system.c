/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "collider_setup-system.h"

#include <stdlib.h>

#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/device/device.h"
#include "core/graphics/mesh/mesh.h"

#include "physics/physics_collision.h"
#include "physics/physics_solver.h"

#define MAX_COLLISION_POINTS         128
#define COLLISION_REQUIRES_RIGIDBODY FALSE

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
    if (collider->type == COLLIDER_SPHERE)
    {
        gsk_SphereCollider *sphereCollider = malloc(sizeof(gsk_SphereCollider));

        // default radius
        sphereCollider->radius = 0.5f;

#if 0
        if (gsk_ecs_has(e, C_MODEL))
        {
            struct ComponentModel *cmp_model = gsk_ecs_get(e, C_MODEL);
            gsk_MeshData *meshdata = ((gsk_Mesh *)cmp_model->mesh)->meshData;

            f32 dist = glm_aabb_radius(meshdata->boundingBox);
            LOG_INFO("RADIUS IS %f", dist);
            sphereCollider->radius = dist / 2;
#if 0
            sphereCollider->radius =
              (dist / 2) * (glm_vec3_norm(transform->scale) / 2);
#endif
        }
#endif

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_SphereCollider *)sphereCollider;
    } else if (collider->type == COLLIDER_PLANE)
    {
        gsk_PlaneCollider *planeCollider = malloc(sizeof(gsk_PlaneCollider));

        // planeCollider->distance      = 0.0175f;
        planeCollider->distance = 10;
        glm_vec3_zero(planeCollider->plane);

        // TODO: Calculate normal from plane mesh with orientation
        vec3 planenorm = {0.0f, 1.0f, 0.0f};
        glm_vec3_copy(planenorm, planeCollider->normal);

        glm_vec3_copy(transform->position, planeCollider->plane);

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_PlaneCollider *)planeCollider;

#if 0
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
#endif
    }

    else if (collider->type == COLLIDER_BOX)
    {
        gsk_BoxCollider *box_collider = malloc(sizeof(gsk_BoxCollider));
        glm_vec3_zero(box_collider->bounds[0]);
        glm_vec3_zero(box_collider->bounds[1]);

        if (gsk_ecs_has(e, C_MODEL))
        {
            struct ComponentModel *cmp_model = gsk_ecs_get(e, C_MODEL);
            gsk_MeshData *meshdata = ((gsk_Mesh *)cmp_model->mesh)->meshData;
            glm_vec3_copy(meshdata->boundingBox[0], box_collider->bounds[0]);
            glm_vec3_copy(meshdata->boundingBox[1], box_collider->bounds[1]);
        }
        // TODO: TESTING
        else if (collider->p_mesh != NULL && collider->p_mesh == 0x32)
        {
            glm_vec3_copy(collider->box_bounds_min, box_collider->bounds[0]);
            glm_vec3_copy(collider->box_bounds_max, box_collider->bounds[1]);

        }
        // TODO: TESTING
        else if (collider->p_mesh != NULL)
        {
            gsk_MeshData *meshdata = ((gsk_Mesh *)collider->p_mesh)->meshData;
            glm_vec3_copy(meshdata->boundingBox[0], box_collider->bounds[0]);
            glm_vec3_copy(meshdata->boundingBox[1], box_collider->bounds[1]);

        }
        // default BOX bounds
        else
        {
            vec3 bounds_min = {-20.0f, -1.0f, -20.0f};
            vec3 bounds_max = {20.0f, 1.0f, 20.0f};
            glm_vec3_copy(bounds_min, box_collider->bounds[0]);
            glm_vec3_copy(bounds_max, box_collider->bounds[1]);
        }

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_BoxCollider *)box_collider;
    }

    else if (collider->type == COLLIDER_CAPSULE)
    {
        gsk_CapsuleCollider *capsule_collider =
          malloc(sizeof(gsk_CapsuleCollider));

        vec3 base  = {0.0f, 1.255f, 0.0f};
        vec3 tip   = {0.0f, 0.5f, 0.0f};
        f32 radius = 0.2f;

        if (e.id >= 313)
        {
            vec3 new_base = {0.0f, -0.2f, 0.0f};
            glm_vec3_copy(new_base, base);
        }

        glm_vec3_copy(base, capsule_collider->base);
        glm_vec3_copy(tip, capsule_collider->tip);
        capsule_collider->radius = radius;

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_CapsuleCollider *)capsule_collider;
    }
}

static void
on_collide(gsk_Entity e)
{
    // test for collisions
    if (!(gsk_ecs_has(e, C_COLLIDER))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

#if COLLISION_REQUIRES_RIGIDBODY
    if (!(gsk_ecs_has(e, C_RIGIDBODY))) return;
#endif // COLLISION_REQUIRES_RIGIDBODY

    struct ComponentCollider *collider   = gsk_ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    collider->isColliding = 0;

    gsk_CollisionPoints points_list[MAX_COLLISION_POINTS];
    gsk_EntityId id_point_list[MAX_COLLISION_POINTS];
    u32 points_list_next = 0;

    for (int i = 0; i < e.ecs->nextIndex; i++)
    {
        if (e.index == (gsk_EntityId)i) continue; // do not check self

        // TODO: fix look-up
        gsk_Entity e_compare = {
          .id    = e.ecs->ids[i],
          .index = (gsk_EntityId)i,
          .ecs   = e.ecs,
        };

        // TODO: Add collision masks
        // if (e.id >= 313 && e_compare.id >= 313) { continue; }

        if (!gsk_ecs_has(e_compare, C_COLLIDER)) continue;
        if (!gsk_ecs_has(e_compare, C_TRANSFORM))
        {
            LOG_WARN("Collider does not have transform (entity %d)",
                     e_compare.id);
            continue;
        }

        struct ComponentCollider *compareCollider =
          gsk_ecs_get(e_compare, C_COLLIDER);

        struct ComponentTransform *compareTransform =
          gsk_ecs_get(e_compare, C_TRANSFORM);

//-----------------------
// parameters used for all collision-check functions
//
// NOTE: this may be a little hacky, but it makes the collision-comparison
// a lot more readable.
#define __clsn_prm                                                 \
    ((gsk_Collider *)collider->pCollider)->collider_data,          \
      ((gsk_Collider *)compareCollider->pCollider)->collider_data, \
      transform->position, compareTransform->position

        //
        // determine which collision-test function to use
        //
        gsk_CollisionPoints points = {.has_collision = 0};

        if (collider->type == COLLIDER_SPHERE)
        { // --- Sphere Collider
            switch (compareCollider->type)
            {
            case COLLIDER_SPHERE:
                points = gsk_physics_collision_find_sphere_sphere(__clsn_prm);
                break;
            case COLLIDER_PLANE:
                points = gsk_physics_collision_find_sphere_plane(__clsn_prm);
                break;
            case COLLIDER_BOX:
                points = gsk_physics_collision_find_sphere_box(__clsn_prm);
                break;
            case COLLIDER_CAPSULE:
                points = gsk_physics_collision_find_sphere_capsule(__clsn_prm);
                break;
            default: break;
            };
        } else if (collider->type == COLLIDER_PLANE)
        { // --- Plane Collider
            switch (compareCollider->type)
            {
            case COLLIDER_SPHERE:
                points = gsk_physics_collision_find_plane_sphere(__clsn_prm);
                break;
            case COLLIDER_PLANE:
            case COLLIDER_BOX:
            default: break;
            };
        } else if (collider->type == COLLIDER_BOX)
        { // --- Box Collider
            switch (compareCollider->type)
            {
            case COLLIDER_SPHERE:
                points = gsk_physics_collision_find_box_sphere(__clsn_prm);
                break;
            case COLLIDER_PLANE:
                points = gsk_physics_collision_find_box_plane(__clsn_prm);
                break;
            case COLLIDER_BOX:
                points = gsk_physics_collision_find_box_box(__clsn_prm);
                break;
#if 1
            case COLLIDER_CAPSULE:
                points = gsk_physics_collision_find_box_capsule(__clsn_prm);
                break;
#endif
            default: break;
            };
        } else if (collider->type == COLLIDER_CAPSULE)
        { // --- Capsule Collider
            switch (compareCollider->type)
            {
            case COLLIDER_PLANE:
                points = gsk_physics_collision_find_capsule_plane(__clsn_prm);
                break;
            case COLLIDER_CAPSULE:
                points = gsk_physics_collision_find_capsule_capsule(__clsn_prm);
                break;
            case COLLIDER_SPHERE:
                points = gsk_physics_collision_find_capsule_sphere(__clsn_prm);
                break;
#if 1
            case COLLIDER_BOX:
                points = gsk_physics_collision_find_capsule_box(__clsn_prm);
                break;
#endif
            default: break;
            }
        }

        // Collision points
        if (points.has_collision && points_list_next < MAX_COLLISION_POINTS)
        {
            collider->isColliding = TRUE;

            points_list[points_list_next] = points;
            id_point_list[points_list_next] =
              (gsk_EntityId)i; // TODO: Should be ID
            points_list_next++;
        }
    }

    for (int i = 0; i < points_list_next; i++)
    {

        // TODO: AGAIN - fix look-up
        gsk_Entity e_compare = {
          .id    = e.ecs->ids[id_point_list[i]],
          .index = (gsk_EntityId)id_point_list[i],
          .ecs   = e.ecs,
        };

        // push collision to rigidbody solver
        if (gsk_ecs_has(e, C_RIGIDBODY))
        {

            struct ComponentRigidbody *rigidbody_a =
              gsk_ecs_get(e, C_RIGIDBODY);

            struct ComponentRigidbody *rigidbody_b = NULL;
            struct ComponentTransform *transform_b = NULL;

            //
            // TODO: Refactor this section to separate function
            //

            // Get body_b Rigidbody
            if (gsk_ecs_has(e_compare, C_RIGIDBODY))
            {
                rigidbody_b = gsk_ecs_get(e_compare, C_RIGIDBODY);
            }

            // Get body_b Transform
            if (gsk_ecs_has(e_compare, C_TRANSFORM))
            {
                transform_b = gsk_ecs_get(e_compare, C_TRANSFORM);
            }

            // calculate inertia
            // TODO: Improve
            f32 inertia = (2.0f / 5.0f); /*  X (mass * radius) */

            // initialize intermediary variables
            vec3 linear_velocity_a, linear_velocity_b   = GLM_VEC3_ZERO_INIT;
            vec3 angular_velocity_a, angular_velocity_b = GLM_VEC3_ZERO_INIT;
            vec3 relative_velocity = GLM_VEC3_ZERO_INIT;
            f32 mass_a, mass_b                       = 0.0f; // default to 1
            f32 inverse_mass_a, inverse_mass_b       = 0.0f; // default to 1
            f32 inertia_a, inertia_b                 = 0.0f;
            f32 inverse_inertia_a, inverse_inertia_b = 0.0f;

            // copy a-values
            glm_vec3_copy(rigidbody_a->linear_velocity, linear_velocity_a);
            glm_vec3_copy(rigidbody_a->angular_velocity, angular_velocity_a);
            mass_a         = rigidbody_a->mass;
            inverse_mass_a = (1.0f / rigidbody_a->mass);

            // TODO: actually calculate HERE
            inertia_a         = inertia * (mass_a * 1.0f);
            inverse_inertia_a = (fabs(inertia_a) > 0.0f) ? 1.0f / inertia_a : 0;

            // copy b-values
            if (rigidbody_b)
            {
                glm_vec3_copy(rigidbody_b->linear_velocity, linear_velocity_b);
                mass_b         = rigidbody_b->mass;
                inverse_mass_b = (1.0f / rigidbody_b->mass);

                // TODO: Actually calculate HERE
                inertia_b = inertia * mass_b * 1;
                inverse_inertia_b =
                  (fabs(inertia_b) > 0) ? 1.0f / inertia_b : 0;

                // calculate relative velocity
                glm_vec3_sub(
                  linear_velocity_a, linear_velocity_b, relative_velocity);

            } else
            {
                glm_vec3_copy(linear_velocity_a, relative_velocity);
            }

            gsk_PhysicsMark mark = {

              .body_a =
                (gsk_DynamicBody) {
                  .mass            = mass_a,
                  .inverse_mass    = inverse_mass_a,
                  .inertia         = inertia_a,
                  .inverse_inertia = inverse_inertia_a,
                },

              .body_b =
                (gsk_DynamicBody) {
                  .mass            = mass_b,
                  .inverse_mass    = inverse_mass_b,
                  .inertia         = inertia_b,
                  .inverse_inertia = inverse_inertia_b,
                },
            };

            // copy vectors
            glm_vec3_copy(linear_velocity_a, mark.body_a.linear_velocity);
            glm_vec3_copy(linear_velocity_b, mark.body_b.linear_velocity);

            glm_vec3_copy(angular_velocity_a, mark.body_a.angular_velocity);
            glm_vec3_copy(angular_velocity_b, mark.body_b.angular_velocity);

            glm_vec3_copy(relative_velocity, mark.relative_velocity);

            // copy world-positions
            glm_vec3_copy(transform->position, mark.body_a.position);
            glm_vec3_copy(transform_b->position, mark.body_b.position);

            // Create a new collision result using our points
            gsk_CollisionResult result = {
              .points       = points_list[i],
              .physics_mark = mark,
              .ent_a_id     = e.id,
              .ent_b_id     = e_compare.id,
            };

            if (points_list_next >= MAX_COLLISION_POINTS)
            {
                LOG_CRITICAL("Max collision points exceeded");
            }
            // Send that over to the rigidbody solver list
            gsk_physics_solver_push((gsk_PhysicsSolver *)rigidbody_a->solver,
                                    result);
        }
    }
}

static void
destroy(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;
    struct ComponentCollider *collider = gsk_ecs_get(entity, C_COLLIDER);

    if (collider->pCollider == NULL) { return; }
    gsk_Collider *p_col = (gsk_Collider *)collider->pCollider;

    if (p_col->collider_data) { free(p_col->collider_data); }

    free(p_col);
}

void
s_collider_setup_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init       = (gsk_ECSSubscriber)init,
                              .on_collide = (gsk_ECSSubscriber)on_collide,
                              .destroy    = (gsk_ECSSubscriber)destroy,
                            }));
}
