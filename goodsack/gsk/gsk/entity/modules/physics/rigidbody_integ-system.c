/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
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

#define GSK_PHYSICS_VELOCITY_ITERATIONS 1
#define GSK_PHYSICS_POSITION_ITERATIONS 1

// Functionality toggles
#define DEBUG_TRACK  0
#define DEBUG_POINTS 0 // 0 -- OFF | value = entity id

#define CALC_INERTIA 1

// physics default values
#define DEFAULT_STATIC_FRICTION  0.6f
#define DEFAULT_DYNAMIC_FRICTION 0.4f

// constant for putting dynamic objects to sleep
#define SLEEP_EPSILON 0.5f

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

    if (rigidbody->is_kinematic == FALSE && collider->is_trigger == FALSE)
    {
        // --
        // -- Add force to linear velocity (ignore mass)
        glm_vec3_add(rigidbody->linear_velocity,
                     rigidbody->force_velocity,
                     rigidbody->linear_velocity);

// Rigidbody sleep threshold
#if 0
    if (glm_vec3_norm(rigidbody->linear_velocity) <= SLEEP_EPSILON &&
        collider->isColliding)
    {
        glm_vec3_zero(rigidbody->force_impulse);
        glm_vec3_zero(rigidbody->force_velocity);
        glm_vec3_zero(rigidbody->torque);

        glm_vec3_zero(rigidbody->angular_velocity);
        glm_vec3_zero(rigidbody->linear_velocity);
        return;
    }
#endif

        // --
        // -- Integrate velocities

        // calculate : position += velocity * delta_time;
        vec3 vD = GLM_VEC3_ZERO_INIT;
        glm_vec3_scale(rigidbody->linear_velocity, delta, vD);

        // update position
        glm_vec3_add(transform->position, vD, transform->position);

        // update orientation
        if (!rigidbody->disable_rotation)
        {

            glm_vec3_add(rigidbody->angular_velocity,
                         rigidbody->torque,
                         rigidbody->angular_velocity);

            vec3 angularDeg; // angularDeg
            glm_vec3_scale(rigidbody->angular_velocity, delta, angularDeg);
            angularDeg[0] = glm_deg(angularDeg[0]);
            angularDeg[1] = glm_deg(angularDeg[1]);
            angularDeg[2] = glm_deg(angularDeg[2]);

            vec3 test = {angularDeg[0], angularDeg[1], angularDeg[2]};

            transform_rotate(transform, test);
        }
    }

    for (int iter = 0; iter < GSK_PHYSICS_POSITION_ITERATIONS; ++iter)
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
                // make sure we set the contact point here
                solver_data.contact_point = j;

                // --
                // Run Solvers

                if (rigidbody->is_kinematic == FALSE &&
                    solver_data.p_collision_result->is_trigger_response ==
                      FALSE)
                {
                    vec3 pos_fix = {0, 0, 0};
                    gsk_physics_position_solver(
                      solver_data,
                      pos_fix); // TODO: don't run many times
                    // add accumulated pos_fix from position_solver
                    glm_vec3_add(
                      transform->position, pos_fix, transform->position);
                }
            }
        }
    }

    // clear solvers

    while (pSolver->solvers_list->is_list_empty == FALSE)
    {
        gsk_physics_solver_pop((gsk_PhysicsSolver *)rigidbody->solver);
    }

    if (pSolver->solvers_list->is_list_empty == FALSE)
    {
        LOG_ERROR("solver list failed to clear on entity %d", entity.id);
    }

    // --
    // -- Reset net forces

    glm_vec3_zero(rigidbody->force_velocity);
    glm_vec3_zero(rigidbody->force_impulse);
    glm_vec3_zero(rigidbody->torque);
}

//-----------------------------------------------------------------------------
void
s_rigidbody_integ_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                            }));
}
//-----------------------------------------------------------------------------