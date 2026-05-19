/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Move solvers to separate file

#include "rigidbody-system.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"
#include "util/vec_colors.h"

#include "tools/debug/debug_context.h"
#include "tools/debug/debug_draw_line.h"

#include "core/device/device.h"
#include "physics/physics_solver.h"

#include "entity/modules/transform/transform.h"

#include "entity/modules/physics/solvers/friction_solver.h"
#include "entity/modules/physics/solvers/position_solver.h"
#include "entity/modules/physics/solvers/solver_data.h"

// Functionality toggles
#define DEBUG_TRACK  0
#define DEBUG_POINTS 0 // 0 -- OFF | value = entity id

#define CALC_INERTIA 1

// physics default values
#define DEFAULT_STATIC_FRICTION  0.6f
#define DEFAULT_DYNAMIC_FRICTION 0.4f

// constant for putting dynamic objects to sleep
#define SLEEP_EPSILON 0.5f

//-----------------------------------------------------------------------------
#if DEBUG_TRACK
static u32 s_dbg_instance       = 0xFF;
static u32 s_dbg_instance_begin = 0xFF;

static void
__debug_ray(const gsk_PhysicsSolverData solver_data,
            vec3 start,
            vec3 direction,
            vec4 color)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_Entity entity                     = solver_data.entity;
    if (DEBUG_POINTS && entity.id == DEBUG_POINTS)
    {
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_RAY,
                               s_dbg_instance,
                               start,
                               direction,
                               1,
                               color,
                               FALSE);
        s_dbg_instance++;
    }
}
#endif

//-----------------------------------------------------------------------------
// TODO: FIX THIS WITH ITERATOR for all points in manifold
static void
__debug_points(const gsk_PhysicsSolverData solver_data)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_Entity entity                     = solver_data.entity;

    if (!(DEBUG_POINTS && entity.id == DEBUG_POINTS)) { return; }

    for (int i = 0; i < solver_data.p_collision_result->manifold.contacts_count;
         i++)
    {

        gsk_CollisionPoints points =
          solver_data.p_collision_result->manifold.contacts[i];

// collision normal
#if 0
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_RAY,
                               entity.id + 25,
                               solver_data.p_transform->position,
                               points.normal,
                               5,
                               VCOL_GREEN,
                               FALSE);
#endif

        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_POINT,
                               entity.id + i,
                               points.point_a,
                               points.normal,
                               5,
                               VCOL_GREEN,
                               FALSE);

        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_POINT,
                               entity.id + ((i + 3) * 2),
                               points.point_b,
                               points.normal,
                               3,
                               VCOL_RED,
                               FALSE);
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
static void
init(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_RIGIDBODY))) return;
    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;

    struct ComponentRigidbody *rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    struct ComponentCollider *collider   = gsk_ecs_get(entity, C_COLLIDER);

#if 0
    glm_vec3_zero(rigidbody->force_impulse);
    glm_vec3_zero(rigidbody->force_velocity);

    glm_vec3_zero(rigidbody->torque);
#endif

    glm_vec3_zero(rigidbody->linear_velocity);
    glm_vec3_zero(rigidbody->angular_velocity);

    // friction constants
    // todo: add restitution here
    rigidbody->static_friction  = DEFAULT_STATIC_FRICTION;
    rigidbody->dynamic_friction = DEFAULT_DYNAMIC_FRICTION;

    if (rigidbody->mass <= 0)
    {
        LOG_WARN("mass was <= 0 - setting mass to 1");
        rigidbody->mass = 1.0f;
    }

#if CALC_INERTIA
    // calculate rotational inertia
    // TODO: Defaults
    f32 inertia = 0;
    if (collider->type == COLLIDER_SPHERE)
    {
        gsk_SphereCollider *p_sphere =
          ((gsk_Collider *)collider->pCollider)->collider_data;

        f32 radius = p_sphere->radius;

        // I = 2/5mr^2 -- solid sphere
        inertia = (2.0f / 5.0f) * rigidbody->mass * (radius * radius);
    }

    else if (collider->type == COLLIDER_BOX)
    {
        // TODO: get width/height from bounds
        f32 width  = 1;
        f32 height = 1;

// I_d = 1/12m(w^2 + h^2) -- rectangular cuboid depth
#if 0
        inertia =
          (1.0f / 12.0f) * rigidbody->mass * (pow(width, 2) + pow(height, 2));
#else
        inertia =
          (1.0f / 12.0f) * rigidbody->mass * (width * width + height * height);
#endif
    }

    else
    {
        inertia = 4.0f;
    }

    rigidbody->inertia = inertia;

    // inverse mass and inertia
    rigidbody->inverse_mass =
      (fabsf(rigidbody->mass) > 0.0f) ? 1.0f / rigidbody->mass : 0.0f;
    rigidbody->inverse_inertia =
      (fabsf(inertia) > 0.0f) ? 1.0f / inertia : 0.0f;
#endif
}

#if 0
static void
__integrate(gsk_Entity entity, gsk_PhysicsSolverLambda lambda)
{
}

static void
fixed_update(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_RIGIDBODY))) return;
    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;
    if (!(gsk_ecs_has(entity, C_TRANSFORM))) return;

    struct ComponentRigidbody *rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    struct ComponentCollider *collider   = gsk_ecs_get(entity, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(entity, C_TRANSFORM);

    // Calculate simulation-time
    const gsk_Time time = gsk_device_getTime();
    const f64 delta     = time.fixed_delta_time * time.time_scale;

    // --
    // -- Check for solvers/collision results

    gsk_PhysicsSolver *pSolver = (gsk_PhysicsSolver *)rigidbody->solver;
    int total_solvers          = (int)pSolver->solvers_list->list_next;

#if DEBUG_TRACK
    s_dbg_instance = s_dbg_instance_begin;
#endif

    u32 total_impulses             = 0;
    gsk_PhysicsSolverLambda lambda = {0};

    for (int iter = 0; iter < GSK_PHYSICS_VELOCITY_ITERATIONS; ++iter)
    {
        for (int i = 0; i < total_solvers; i++)
        {
            gsk_CollisionResult *pResult = &pSolver->solvers[i];

            // --
            // construct solver_data used to pass into solver functions
            gsk_PhysicsSolverData solver_data = {
              //.p_rigidbody        = rigidbody,
              //.p_transform        = transform,
              .p_collision_result = pResult,
              //.entity             = entity,
              .delta         = delta,
              .contact_point = 0,
            };

            for (int j = 0; j < pResult->manifold.contacts_count; j++)
            {
                // total_impulses++;
                // make sure we set the contact point here
                solver_data.contact_point = j;

                // --
                // Run Solvers

                if (solver_data.p_collision_result->is_trigger_response == TRUE)
                {
                    continue;
                }

                // run the solver
                gsk_PhysicsSolverLambda output = gsk_physics_impulse_solver(
                  solver_data, pResult->manifold.contacts[j].lambda_n);

                // adjust lambda
                glm_vec3_add(
                  lambda.impulse_a, output.impulse_a, lambda.impulse_a);
                glm_vec3_add(
                  lambda.impulse_b, output.impulse_b, lambda.impulse_b);

                glm_vec3_add(lambda.torque_a, output.torque_a, lambda.torque_a);
                glm_vec3_add(lambda.torque_b, output.torque_b, lambda.torque_b);

                lambda.lambda_n                        = output.lambda_n;
                pResult->manifold.contacts[j].lambda_n = output.lambda_n;

                // probably just update actual velocity instead?

                glm_vec3_add(lambda.impulse_a,
                             pResult->physics_mark.body_a.linear_velocity,
                             pResult->physics_mark.body_a.linear_velocity);
                glm_vec3_add(lambda.torque_a,
                             pResult->physics_mark.body_a.angular_velocity,
                             pResult->physics_mark.body_a.angular_velocity);

#if 1
                glm_vec3_add(lambda.impulse_b,
                             pResult->physics_mark.body_b.linear_velocity,
                             pResult->physics_mark.body_b.linear_velocity);
                glm_vec3_add(lambda.torque_b,
                             pResult->physics_mark.body_b.angular_velocity,
                             pResult->physics_mark.body_b.angular_velocity);
#endif

                // WAIT IT MAKES SENSE
                // EVERY CONTACT POINT STORES THE LAMBDA FOR THE NEXT ITERATION
                // if it's too much, it doesn't do anything ,right?

                // TODO: probably apply here?

                // --
                // Run debug markers

#if DEBUG_POINTS
                __debug_points(solver_data);
#endif // DEBUG_POINTS
            }

            // CHECK JUST IN CASE
            // integrate for B
            struct ComponentRigidbody *tgt_rigidbody = NULL;
            gsk_Entity ent_tgt = gsk_ecs_ent(entity.ecs, pResult->ent_b_id);
            if (gsk_ecs_has(ent_tgt, C_RIGIDBODY))
            {
                tgt_rigidbody = gsk_ecs_get(ent_tgt, C_RIGIDBODY);
                glm_vec3_add(lambda.impulse_b,
                             tgt_rigidbody->force_velocity,
                             tgt_rigidbody->force_velocity);

                glm_vec3_add(lambda.torque_b,
                             tgt_rigidbody->torque,
                             tgt_rigidbody->torque);
            }

            glm_vec3_add(lambda.impulse_a,
                         rigidbody->force_velocity,
                         rigidbody->force_velocity);
            glm_vec3_add(lambda.torque_a, rigidbody->torque, rigidbody->torque);
        }
    }
}
#endif

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
s_rigidbody_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init = (gsk_ECSSubscriber)init,
                            }));
}
//-----------------------------------------------------------------------------