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

    gsk_CollisionManifold manifold_list[MAX_COLLISION_POINTS];
    gsk_EntityId id_manifold_list[MAX_COLLISION_POINTS];
    u32 manifold_list_next = 0;

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
        if (gsk_ecs_has(e_compare, C_RIGIDBODY))
        {
            if (gsk_physics_solver_exists(e.id, e_compare.id)) { continue; }
        }

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
                mat3 brot_s = GLM_MAT3_IDENTITY_INIT;
                glm_mat4_pick3(compareTransform->m4_rotation, brot_s);
                points =
                  gsk_physics_collision_find_sphere_box(__clsn_prm, brot_s);
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
                mat3 arot_s = GLM_MAT3_IDENTITY_INIT;
                glm_mat4_pick3(transform->m4_rotation, arot_s);
                points =
                  gsk_physics_collision_find_box_sphere(__clsn_prm, arot_s);
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
            // TODO: check that this belongs here
            if (collider->is_trigger == TRUE) { continue; }

            collider->isColliding        = TRUE;
            compareCollider->isColliding = TRUE;

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

        //
        // TODO: Refactor this section to separate function
        //

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
            glm_vec3_copy(rigidbody_b->angular_velocity, angular_velocity_b);
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
            inverse_mass_b    = mass_b;
            inverse_inertia_b = (fabsf(inertia_b) > 0) ? (inertia_b * 0.5f) : 0;
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