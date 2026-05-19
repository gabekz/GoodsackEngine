/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "collider_setup-system.h"

#include <stdlib.h>

#include "util/maths.h"
#include "util/sysdefs.h"

#include "entity/ecs.h"

#include "core/device/device.h"
#include "core/graphics/mesh/mesh.h"

#include "physics/physics_collision.h"
#include "physics/physics_solver.h"

#include "runtime/gsk_runtime_wrapper.h"

#define MAX_COLLISION_POINTS         128
#define COLLISION_REQUIRES_RIGIDBODY FALSE

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_COLLIDER))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCollider *collider   = gsk_ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    collider->isColliding = FALSE; // TODO: remove <- this is for testing

    collider->pCollider = (gsk_Collider *)malloc(sizeof(gsk_Collider));
    ((gsk_Collider *)collider->pCollider)->collider_data_type = collider->type;

    //(gsk_Collider *)(collider->pCollider).position = transform->position;

    // TODO: collider types
    if (collider->type == COLLIDER_SPHERE)
    {
        gsk_SphereCollider *sphereCollider = malloc(sizeof(gsk_SphereCollider));

        if (collider->radius <= 0)
        {
            // default radius
            collider->radius = 0.5f;
        }

        sphereCollider->radius = collider->radius;

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

        if (gsk_ecs_has(e, C_MODEL) && collider->p_mesh != 0x32)
        {
            struct ComponentModel *cmp_model = gsk_ecs_get(e, C_MODEL);
            gsk_MeshData *meshdata = ((gsk_Mesh *)cmp_model->mesh)->meshData;
            glm_vec3_copy(meshdata->boundingBox[0], box_collider->bounds[0]);
            glm_vec3_copy(meshdata->boundingBox[1], box_collider->bounds[1]);

        }
        // TODO: maybe pass in the center as the body position for the friction
        // solver or solver_data?
        // TODO: TESTING
        else if (collider->p_mesh == 0x32)
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
            LOG_WARN("no mesh found found box collider on entity %d. Setting "
                     "default bounds",
                     e.id);

            vec3 bounds_min = {-1.0f, -1.0f, -1.0f};
            vec3 bounds_max = {1.0f, 1.0f, 1.0f};
            glm_vec3_copy(bounds_min, box_collider->bounds[0]);
            glm_vec3_copy(bounds_max, box_collider->bounds[1]);
        }

#if 1
        glm_vec3_mul(
          box_collider->bounds[0], transform->scale, box_collider->bounds[0]);
        glm_vec3_mul(
          box_collider->bounds[1], transform->scale, box_collider->bounds[1]);
#endif

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

// TODO: CHANGE THIS - temp for secondary capsule test
// TODO: TODAY
#if 1
        if (e.id >= 304)
        {
            vec3 new_base = {0.0f, -0.2f, 0.0f};
            vec3 new_tip  = {0.0f, 1.5f, 0.0f};
            glm_vec3_copy(new_base, base);
            glm_vec3_copy(new_tip, tip);
        }
#endif

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

    gsk_CollisionManifold manifold_list[MAX_COLLISION_POINTS];
    gsk_EntityId id_manifold_list[MAX_COLLISION_POINTS];
    u32 manifold_list_next = 0;

#if 0
    if (gsk_ecs_has(e, C_RIGIDBODY))
    {
        struct ComponentRigidbody *rigidbody = gsk_ecs_get(e, C_RIGIDBODY);

        gsk_PhysicsSolver *pSolver = (gsk_PhysicsSolver *)rigidbody->solver;
        int total_solvers          = (int)pSolver->solvers_list->list_next;

        if (total_solvers > 0) { LOG_ERROR("HAS MORE THAN 1"); }
    }
#endif

    if (gsk_ecs_has(e, C_RIGIDBODY))
    {
        struct ComponentRigidbody *rigidbody = gsk_ecs_get(e, C_RIGIDBODY);
        if (rigidbody->is_kinematic) { return; }
    }

    for (int i = 0; i < e.ecs->nextIndex; i++)
    {
        if (e.id == e.ecs->p_ent_ids[i]) continue; // do not check self

        gsk_Entity e_compare =
          gsk_ecs_ent(e.ecs, (gsk_EntityId)e.ecs->p_ent_ids[i]);

        // IMPORTANT:
        // check to see if we already have a collision manifold.
        if (gsk_ecs_has(e_compare, C_RIGIDBODY))
        {
            struct ComponentRigidbody *tgt_rigidbody =
              gsk_ecs_get(e_compare, C_RIGIDBODY);

            if (gsk_physics_solver_exists(e.id, e_compare.id)) { continue; }
        }

        // check layer mask to ensure collision is allowed
        if (!gsk_runtime_check_layer_mask(e.ecs->p_ent_layers[e.index],
                                          e.ecs->p_ent_layers[e_compare.index]))
        {
            continue;
        }

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
      transform->world_position, compareTransform->world_position

        //
        // determine which collision-test function to use
        //

        gsk_CollisionPoints points     = {0};
        gsk_CollisionManifold manifold = {0};

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
                mat3 arot = GLM_MAT3_IDENTITY_INIT;
                mat3 brot = GLM_MAT3_IDENTITY_INIT;
                glm_mat4_pick3(transform->m4_rotation, arot);
                glm_mat4_pick3(compareTransform->m4_rotation, brot);
                manifold =
                  gsk_physics_collision_find_box_box(__clsn_prm, arot, brot);
                break;
            case COLLIDER_CAPSULE:
                points = gsk_physics_collision_find_box_capsule(__clsn_prm);
                break;
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
            case COLLIDER_BOX:
                points = gsk_physics_collision_find_capsule_box(__clsn_prm);
                break;
            default: break;
            }
        }

        // TODO: this is a hack for now just to test manifolds
        if ((collider->type == COLLIDER_BOX &&
             compareCollider->type == COLLIDER_BOX) == FALSE)
        {
            manifold.contacts_count = 1;
            manifold.contacts[0]    = points;
            manifold.depth          = points.depth;
            manifold.has_collision  = points.has_collision;
            glm_vec3_copy(points.normal, manifold.normal);
        }

        if (manifold.has_collision &&
            manifold_list_next >= MAX_COLLISION_POINTS)
        {
            LOG_ERROR("MAX COLLISION POINTS");
        }

        // Collision points
        if (manifold.has_collision && manifold_list_next < MAX_COLLISION_POINTS)
        {

            collider->isColliding = TRUE;

#if 0
            const char *luaCode = "hook.Run(\"PerformMultiplication\", 6, 4)";
            luaL_dostring(L, luaCode);
#endif

#if 1
            // skip this entity if it is a trigger
            if (collider->is_trigger == TRUE)
            {
                // LOG_INFO("TRIGGER");
                continue;
            }
#endif

            manifold_list[manifold_list_next] = manifold;
            id_manifold_list[manifold_list_next] =
              (gsk_EntityId)i; // TODO: Should be ID
            manifold_list_next++;
        }
    }

    for (int i = 0; i < manifold_list_next; i++)
    {
        gsk_Entity e_compare = gsk_ecs_ent(
          e.ecs, (gsk_EntityId)e.ecs->p_ent_ids[id_manifold_list[i]]);

        struct ComponentCollider *compareCollider =
          gsk_ecs_get(e_compare, C_COLLIDER);

        // push collision to rigidbody solver
        if (gsk_ecs_has(e, C_RIGIDBODY))
        {
            struct ComponentRigidbody *rigidbody_a =
              gsk_ecs_get(e, C_RIGIDBODY);

            struct ComponentRigidbody *rigidbody_b = NULL;
            struct ComponentTransform *transform_b = NULL;

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

#if 1
            if (compareCollider->is_trigger == TRUE)
            {
                // Create a new collision result using our points
                gsk_CollisionResult result = {
                  .manifold = manifold_list[i], // TODO: possibly invert points
                  .physics_mark        = (gsk_PhysicsMark) {0},
                  .ent_a_id            = e_compare.id,
                  .ent_b_id            = e.id,
                  .is_trigger_response = TRUE,
                };

                if (rigidbody_b != NULL)
                {

                    // Send that over to rigidbody (B) solver list
                    // TODO: this is going to be broken with kinematic-ness
                    gsk_physics_solver_push(result);
                }

                // skip this comparison because we don't want to walk on
                // triggers
                continue;
            }
#endif

            //
            // TODO: Refactor this section to separate function
            //

            // calculate inertia
            // TODO: Improve
            f32 inertia = (2.0f / 5.0f); /*  X (mass * radius) */

            // initialize intermediary variables
            vec3 linear_velocity_a, linear_velocity_b   = GLM_VEC3_ZERO_INIT;
            vec3 angular_velocity_a, angular_velocity_b = GLM_VEC3_ZERO_INIT;
            vec3 relative_velocity = GLM_VEC3_ZERO_INIT;

            f32 mass_a, mass_b                         = 0.0f; // default to 1
            f32 inverse_mass_a, inverse_mass_b         = 0.0f; // default to 1
            f32 inertia_a, inertia_b                   = 0.0f;
            f32 inverse_inertia_a, inverse_inertia_b   = 0.0f;
            f32 static_friction_a, static_friction_b   = 0.0f;
            f32 dynamic_friction_a, dynamic_friction_b = 0.0f;

            // copy a-values
            glm_vec3_copy(rigidbody_a->linear_velocity, linear_velocity_a);
            glm_vec3_copy(rigidbody_a->angular_velocity, angular_velocity_a);

            mass_a         = rigidbody_a->mass;
            inverse_mass_a = rigidbody_a->inverse_mass;

            inertia_a         = rigidbody_a->inertia;
            inverse_inertia_a = rigidbody_a->inverse_inertia;

            static_friction_a  = rigidbody_a->static_friction;
            dynamic_friction_a = rigidbody_a->dynamic_friction;

            // copy b-values
            if (rigidbody_b == NULL)
            {
                glm_vec3_copy(linear_velocity_a, relative_velocity);

            }
            // copy b-values for standard rigidbodies
            else if (rigidbody_b->is_kinematic == FALSE)
            {
                glm_vec3_copy(rigidbody_b->linear_velocity, linear_velocity_b);
                glm_vec3_copy(rigidbody_b->angular_velocity,
                              angular_velocity_b);
                mass_b         = rigidbody_b->mass;
                inverse_mass_b = rigidbody_b->inverse_mass;

                inertia_b         = rigidbody_b->inertia;
                inverse_inertia_b = rigidbody_b->inverse_inertia;

                static_friction_b  = rigidbody_b->static_friction;
                dynamic_friction_b = rigidbody_b->dynamic_friction;

                // calculate relative velocity
                glm_vec3_sub(
                  linear_velocity_a, linear_velocity_b, relative_velocity);
            }

#if 1
            else
            {
                inverse_mass_b = mass_b * 0.5f;
                inverse_inertia_b =
                  (fabsf(inertia_b) > 0) ? (inertia_b * 0.5f) : 0;
            }
#endif

            gsk_PhysicsMark mark = {

              .body_a =
                (gsk_DynamicBody) {
                  .mass             = mass_a,
                  .inverse_mass     = inverse_mass_a,
                  .inertia          = inertia_a,
                  .inverse_inertia  = inverse_inertia_a,
                  .static_friction  = static_friction_a,
                  .dynamic_friction = dynamic_friction_a,
                },

              .body_b =
                (gsk_DynamicBody) {
                  .mass             = mass_b,
                  .inverse_mass     = inverse_mass_b,
                  .inertia          = inertia_b,
                  .inverse_inertia  = inverse_inertia_b,
                  .static_friction  = static_friction_b,
                  .dynamic_friction = dynamic_friction_b,
                },
            };

            // copy vectors
            glm_vec3_copy(linear_velocity_a, mark.body_a.linear_velocity);
            glm_vec3_copy(linear_velocity_b, mark.body_b.linear_velocity);

            glm_vec3_copy(angular_velocity_a, mark.body_a.angular_velocity);
            glm_vec3_copy(angular_velocity_b, mark.body_b.angular_velocity);

            glm_vec3_copy(relative_velocity, mark.relative_velocity);

            // copy world-positions
            glm_vec3_copy(transform->world_position, mark.body_a.position);
            glm_vec3_copy(transform_b->world_position, mark.body_b.position);

            // Create a new collision result using our points
            gsk_CollisionResult result = {
              .manifold            = manifold_list[i],
              .physics_mark        = mark,
              .ent_a_id            = e.id,
              .ent_b_id            = e_compare.id,
              .is_trigger_response = FALSE,
            };

            if (manifold_list_next >= MAX_COLLISION_POINTS)
            {
                LOG_CRITICAL("Max collision points exceeded");
            }
            // Send that over to the rigidbody solver list
            gsk_physics_solver_push(result);
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
