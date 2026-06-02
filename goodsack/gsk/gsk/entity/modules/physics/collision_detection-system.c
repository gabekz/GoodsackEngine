/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "collision_detection-system.h"

#define MAX_COLLISION_POINTS         128
#define COLLISION_REQUIRES_RIGIDBODY FALSE

#include "entity/ecs.h"

#include "core/device/device.h"
#include "core/graphics/mesh/mesh.h"

#include "physics/physics_collision.h"
#include "physics/physics_solver.h"

#include "runtime/gsk_runtime_wrapper.h"

#include "entity/modules/physics/physics_util.h"
#include "entity/modules/transform/transform.h"

#include "util/maths.h"
#include "util/sysdefs.h"

static void
_singleton_clear_collider_status(gsk_ECS *p_ecs)
{
    for (int i = 0; i < p_ecs->nextIndex; i++)
    {
        gsk_Entity ent = gsk_ecs_ent(p_ecs, (gsk_EntityId)p_ecs->p_ent_ids[i]);

        if (!gsk_ecs_has(ent, C_COLLIDER)) { continue; }
        gsk_C_Collider *cmp_collider = gsk_ecs_get(ent, C_COLLIDER);

        cmp_collider->isColliding = FALSE;
    }
}

//-----------------------------------------------------------------------------
static void
on_collide(gsk_Entity e)
{
    // NOTE: important step here to reset collision status
    if (e.index == 0) { _singleton_clear_collider_status(e.ecs); }

    // test for collisions
    if (!(gsk_ecs_has(e, C_COLLIDER))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

#if COLLISION_REQUIRES_RIGIDBODY
    if (!(gsk_ecs_has(e, C_RIGIDBODY))) return;
#endif // COLLISION_REQUIRES_RIGIDBODY

    struct ComponentCollider *collider   = gsk_ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

#if 0
    // TODO: make this a function
    if (collider->type == COLLIDER_BOX)
    {
        glm_mat4_pick3(
          transform->m4_rotation((gsk_BoxCollider *)collider->pCollider)
            ->rotation);
    }
#endif

    if (gsk_ecs_has(e, C_RIGIDBODY))
    {
        struct ComponentRigidbody *rigidbody = gsk_ecs_get(e, C_RIGIDBODY);

        // NOTE: might want to make this optional?
        if (rigidbody->is_kinematic) { return; }
    }

    for (int i = 0; i < e.ecs->nextIndex; i++)
    {
        // ignore self
        if (e.id == e.ecs->p_ent_ids[i]) { continue; }

        gsk_Entity e_compare =
          gsk_ecs_ent(e.ecs, (gsk_EntityId)e.ecs->p_ent_ids[i]);

        // IMPORTANT:
        // check to see if we already have a collision manifold.
        if (gsk_physics_solver_exists(e.id, e_compare.id)) { continue; }

        // check layer mask to ensure collision is allowed
        if (!gsk_runtime_check_layer_mask(e.ecs->p_ent_layers[e.index],
                                          e.ecs->p_ent_layers[e_compare.index]))
        {
            continue;
        }

        if (!gsk_ecs_has(e_compare, C_COLLIDER)) { continue; }
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
        // NOTE: this may be a little hacky, but it makes the
        // collision-comparison a lot more readable.

        vec3 a, b;
        vec3 pos_a, pos_b;

#if 0
        transform_point_local_to_world(
          transform, collider->center, collider->new_extents);
        transform_point_local_to_world(compareTransform,
                                       compareCollider->center,
                                       compareCollider->newExtents);
#elif 0
        glm_vec3_add(transform->world_position, collider->center, pos_a);
        glm_vec3_add(
          compareTransform->world_position, compareCollider->center, pos_b);
#else
        glm_vec3_copy(transform->world_position, pos_a);
        glm_vec3_copy(compareTransform->world_position, pos_b);

#endif

#define __clsn_prm                                                        \
    ((gsk_Collider *)collider->pCollider)->collider_data,                 \
      ((gsk_Collider *)compareCollider->pCollider)->collider_data, pos_a, \
      pos_b

        // TODO: world_position not working when changing this to fixed_update

        //
        // determine which collision-test function to use
        //

        gsk_CollisionPoints points     = {0};
        gsk_CollisionManifold manifold = {0};

        mat3 a_rot = GLM_MAT3_IDENTITY_INIT;
        mat3 b_rot = GLM_MAT3_IDENTITY_INIT;

        glm_quat_mat3(transform->rotation, a_rot);
        glm_quat_mat3(compareTransform->rotation, b_rot);

        // glm_mat4_pick3(transform->m4_rotation, a_rot);
        // glm_mat4_pick3(compareTransform->m4_rotation, b_rot);

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
                points =
                  gsk_physics_collision_find_sphere_box(__clsn_prm, b_rot);
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
                points =
                  gsk_physics_collision_find_box_sphere(__clsn_prm, a_rot);
                break;
            case COLLIDER_PLANE:
                points = gsk_physics_collision_find_box_plane(__clsn_prm);
                break;
            case COLLIDER_BOX:
                manifold =
                  gsk_physics_collision_find_box_box(__clsn_prm, a_rot, b_rot);
                break;
            case COLLIDER_CAPSULE:
                points =
                  gsk_physics_collision_find_box_capsule(__clsn_prm, a_rot);
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
                points =
                  gsk_physics_collision_find_capsule_box(__clsn_prm, b_rot);
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

        // Collision points
        if (manifold.has_collision)
        {
            // TODO: check that this belongs here
            if (collider->is_trigger == TRUE) { continue; }

            collider->isColliding        = TRUE;
            compareCollider->isColliding = TRUE;

            // calculate local points
            for (int pt = 0; pt < manifold.contacts_count; pt++)
            {
                gsk_CollisionPoints *p_contact = &manifold.contacts[pt];
                transform_point_world_to_local(
                  transform, p_contact->point_a, p_contact->local_point_a);
                transform_point_world_to_local(compareTransform,
                                               p_contact->point_b,
                                               p_contact->local_point_b);
            }

#if 0
            manifold_list[manifold_list_next] = manifold;
            id_manifold_list[manifold_list_next] =
              (gsk_EntityId)i; // TODO: Should be ID
            manifold_list_next++;
#else
#if 1
            if (compareCollider->is_trigger == TRUE)
            {
                // Create a new collision result using our points
                gsk_CollisionResult result = {
                  .manifold            = manifold,
                  .physics_mark        = (gsk_PhysicsMark) {0},
                  .ent_a_id            = e_compare.id,
                  .ent_b_id            = e.id,
                  .is_trigger_response = TRUE,
                };

                // TODO: CHECK if this is going to be broken with kinematic-ness
                gsk_physics_solver_push(result);

                // skip this comparison because we don't want to walk on
                // triggers
                continue;
            }
#endif

            // Create a new collision result using our points
            gsk_CollisionResult result = {
              .manifold = manifold,
              .physics_mark =
                gsk_physics_util_create_physics_mark(e, e_compare),
              .ent_a_id            = e.id,
              .ent_b_id            = e_compare.id,
              .is_trigger_response = FALSE,
            };

            // Send that over to the rigidbody solver list
            gsk_physics_solver_push(result);
#endif
        }
    }

#if 0
    for (int i = 0; i < manifold_list_next; i++)
    {
        gsk_Entity e_compare = gsk_ecs_ent(
          e.ecs, (gsk_EntityId)e.ecs->p_ent_ids[id_manifold_list[i]]);

        struct ComponentCollider *compareCollider =
          gsk_ecs_get(e_compare, C_COLLIDER);

        // TODO: make sure this doesn't break shit
        // push collision to rigidbody solver
        if (!gsk_ecs_has(e, C_RIGIDBODY)) { continue; }

        struct ComponentRigidbody *rigidbody_a = gsk_ecs_get(e, C_RIGIDBODY);

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
              .manifold     = manifold_list[i], // TODO: possibly invert points
              .physics_mark = (gsk_PhysicsMark) {0},
              .ent_a_id     = e_compare.id,
              .ent_b_id     = e.id,
              .is_trigger_response = TRUE,
            };

            // TODO: CHECK if this is going to be broken with kinematic-ness
            gsk_physics_solver_push(result);

            // skip this comparison because we don't want to walk on
            // triggers
            continue;
        }
#endif

        gsk_PhysicsMark mark =
          gsk_physics_util_create_physics_mark(e, e_compare);

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
#endif
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
s_collision_detection_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .on_collide = (gsk_ECSSubscriber)on_collide,
                            }));
}
//-----------------------------------------------------------------------------