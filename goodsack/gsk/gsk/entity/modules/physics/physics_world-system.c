/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_world-system.h"

#include "entity/ecs.h"
#include "entity/ecsdefs.h"

#include "physics/physics_solver.h"
#include "physics/physics_types.h"

#include "entity/modules/physics/solvers/friction_solver.h"
#include "entity/modules/physics/solvers/position_solver.h"
#include "entity/modules/physics/solvers/solver_data.h"

#include "core/device/device.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#define _VELOCITY_ITERATIONS 6
#define _FRICTION_ITERATIONS 3
#define _POSITION_ITERATIONS 1

static void
__apply_gravity(gsk_Entity entity, f64 delta)
{
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return; }

    if (delta > 0)
    {
        glm_vec3_divs(
          cmp_rigidbody->force_velocity, delta, cmp_rigidbody->force_velocity);
    }

    // --
    // -- Add gravity to net force (mass considered)

    // mass * gravity
    vec3 mG = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(cmp_rigidbody->gravity, cmp_rigidbody->mass, mG);
    glm_vec3_add(
      cmp_rigidbody->force_velocity, mG, cmp_rigidbody->force_velocity);

    // --
    // -- Add net force to velocity (mass considered)

    // velocity += force / mass * delta_time;
    vec3 fDm = GLM_VEC3_ZERO_INIT;
    glm_vec3_divs(cmp_rigidbody->force_velocity, cmp_rigidbody->mass, fDm);
    glm_vec3_scale(fDm, delta, cmp_rigidbody->force_velocity);
}

static u8
__apply_linear_velocity(gsk_Entity entity, vec3 impulse)
{
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return FALSE; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return FALSE; }
    // TODO: scale here later rather than from solver

    glm_vec3_add(
      cmp_rigidbody->linear_velocity, impulse, cmp_rigidbody->linear_velocity);

    return TRUE;
}

static u8
__apply_angular_velocity(gsk_Entity entity, vec3 torque)
{
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return FALSE; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return FALSE; }

    if (cmp_rigidbody->disable_rotation) { return FALSE; }

    glm_vec3_add(
      cmp_rigidbody->angular_velocity, torque, cmp_rigidbody->angular_velocity);

    return TRUE;
}

static void
__integrate_velocity(gsk_Entity entity, f64 delta)
{
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return; }

    gsk_C_Transform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    // --
    // -- Add force to linear velocity (ignore mass)
    glm_vec3_add(cmp_rigidbody->linear_velocity,
                 cmp_rigidbody->force_velocity,
                 cmp_rigidbody->linear_velocity);

    // calculate : position += velocity * delta_time;
    vec3 vD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(cmp_rigidbody->linear_velocity, delta, vD);

    // update position
    glm_vec3_add(cmp_transform->position, vD, cmp_transform->position);

    // update orientation
    if (!cmp_rigidbody->disable_rotation)
    {

        glm_vec3_add(cmp_rigidbody->angular_velocity,
                     cmp_rigidbody->torque,
                     cmp_rigidbody->angular_velocity);

        vec3 angularDeg; // angularDeg
        glm_vec3_scale(cmp_rigidbody->angular_velocity, delta, angularDeg);
        angularDeg[0] = glm_deg(angularDeg[0]);
        angularDeg[1] = glm_deg(angularDeg[1]);
        angularDeg[2] = glm_deg(angularDeg[2]);

        vec3 test = {angularDeg[0], angularDeg[1], angularDeg[2]};

        transform_rotate(cmp_transform, test);
    }

    // zero-out forces

    glm_vec3_zero(cmp_rigidbody->force_velocity);
    glm_vec3_zero(cmp_rigidbody->force_impulse);
    glm_vec3_zero(cmp_rigidbody->torque);
}
static void
__integrate_position_add(gsk_Entity entity, vec3 new_pos)
{
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return; }

    gsk_C_Transform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    glm_vec3_add(cmp_transform->position, new_pos, cmp_transform->position);
}

//-----------------------------------------------------------------------------
static void
init(gsk_Entity entity)
{
    // NOTE: hack so this only runs on one entity.
    // should have proper support for singleton ECS systems.
    if (entity.index != 0) { return; }

    // intialize solver
    gsk_physics_solver_init();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
static void
fixed_update(gsk_Entity entity)
{
    // NOTE: hack so this only runs on one entity.
    // should have proper support for singleton ECS systems.
    if (entity.index != 0) { return; }

    /*==== Setup =====================================================*/

    const gsk_Time time = gsk_device_getTime();
    const f64 delta     = time.fixed_delta_time * time.time_scale;

    gsk_PhysicsSolver *p_solver = gsk_physics_solver_get();
    u8 total_solvers            = p_solver->solvers_list->list_next;

    /*==== Velocity Impulse Solver ===================================*/

    for (int iter = 0; iter < _VELOCITY_ITERATIONS; ++iter)
    {
        for (int i_solver = 0; i_solver < total_solvers; ++i_solver)
        {
            gsk_CollisionResult *pResult = &(p_solver->solvers[i_solver]);

            // --
            // construct solver_data used to pass into solver functions
            gsk_PhysicsSolverData solver_data = {
              .p_collision_result = pResult,
              .delta              = delta,
              .contact_point      = 0,
              .entity             = entity,
            };

            gsk_Entity entity_a =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_a_id);
            gsk_Entity entity_b =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_b_id);

            for (int j = 0; j < pResult->manifold.contacts_count; j++)
            {
                solver_data.contact_point = j;

                if (solver_data.p_collision_result->is_trigger_response == TRUE)
                {
                    continue;
                }

                // run the solver
                gsk_PhysicsSolverLambda output = gsk_physics_impulse_solver(
                  solver_data, pResult->manifold.contacts[j].lambda_n);

                pResult->manifold.contacts[j].lambda_n = output.lambda_n;

                // Apply Velocity to Body A

                if (__apply_linear_velocity(entity_a, output.impulse_a))
                {

                    glm_vec3_add(pResult->physics_mark.body_a.linear_velocity,
                                 output.impulse_a,
                                 pResult->physics_mark.body_a.linear_velocity);
                }

                if (__apply_angular_velocity(entity_a, output.torque_a))
                {
                    glm_vec3_add(pResult->physics_mark.body_a.angular_velocity,
                                 output.torque_a,
                                 pResult->physics_mark.body_a.angular_velocity);
                }

                // Apply Velocity to Body B

                if (__apply_linear_velocity(entity_b, output.impulse_b))
                {

                    glm_vec3_add(pResult->physics_mark.body_b.linear_velocity,
                                 output.impulse_b,
                                 pResult->physics_mark.body_b.linear_velocity);
                }

                if (__apply_angular_velocity(entity_b, output.torque_b))
                {
                    glm_vec3_add(pResult->physics_mark.body_b.angular_velocity,
                                 output.torque_b,
                                 pResult->physics_mark.body_b.angular_velocity);
                }
            }
        }
    }

#if 1
    /*==== FRICTION Impulse Solver ===================================*/

    for (int iter = 0; iter < _FRICTION_ITERATIONS; ++iter)
    {
        for (int i_solver = 0; i_solver < total_solvers; ++i_solver)
        {
            gsk_CollisionResult *pResult = &(p_solver->solvers[i_solver]);

            // --
            // construct solver_data used to pass into solver functions
            gsk_PhysicsSolverData solver_data = {
              .p_collision_result = pResult,
              .delta              = delta,
              .contact_point      = 0,
              .entity             = entity,
            };

            gsk_Entity entity_a =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_a_id);
            gsk_Entity entity_b =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_b_id);

            for (int j = 0; j < pResult->manifold.contacts_count; j++)
            {
                solver_data.contact_point = j;

                if (solver_data.p_collision_result->is_trigger_response == TRUE)
                {
                    continue;
                }

                // run the solver
                gsk_PhysicsSolverLambda output = gsk_physics_friction_solver(
                  solver_data,
                  pResult->manifold.contacts[j].lambda_n,
                  pResult->manifold.contacts[j].lambda_t);

                pResult->manifold.contacts[j].lambda_t = output.lambda_t;

                // Apply Velocity to Body A

                if (__apply_linear_velocity(entity_a, output.impulse_a))
                {

                    glm_vec3_add(pResult->physics_mark.body_a.linear_velocity,
                                 output.impulse_a,
                                 pResult->physics_mark.body_a.linear_velocity);
                }

                if (__apply_angular_velocity(entity_a, output.torque_a))
                {
                    glm_vec3_add(pResult->physics_mark.body_a.angular_velocity,
                                 output.torque_a,
                                 pResult->physics_mark.body_a.angular_velocity);
                }

                // Apply Velocity to Body B

                if (__apply_linear_velocity(entity_b, output.impulse_b))
                {

                    glm_vec3_add(pResult->physics_mark.body_b.linear_velocity,
                                 output.impulse_b,
                                 pResult->physics_mark.body_b.linear_velocity);
                }

                if (__apply_angular_velocity(entity_b, output.torque_b))
                {
                    glm_vec3_add(pResult->physics_mark.body_b.angular_velocity,
                                 output.torque_b,
                                 pResult->physics_mark.body_b.angular_velocity);
                }
            }
        }
    }
#endif

    /*==== Integrate velocity ========================================*/

    for (int i = 0; i < entity.ecs->nextIndex; i++)
    {
        gsk_Entity ent =
          gsk_ecs_ent(entity.ecs, (gsk_EntityId)entity.ecs->p_ent_ids[i]);

        __apply_gravity(ent, delta);
        __integrate_velocity(ent, delta);
    }

    /*==== Position Solver ===========================================*/

    for (int iter = 0; iter < _POSITION_ITERATIONS; ++iter)
    {
        for (int i_solver = 0; i_solver < total_solvers; ++i_solver)
        {
            gsk_CollisionResult *pResult = &(p_solver->solvers[i_solver]);

            // --
            // construct solver_data used to pass into solver functions
            gsk_PhysicsSolverData solver_data = {
              .p_collision_result = pResult,
              .delta              = delta,
              .contact_point      = 0,
              .entity             = entity,
            };

            gsk_Entity entity_a =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_a_id);
            gsk_Entity entity_b =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_b_id);

            for (int j = 0; j < pResult->manifold.contacts_count; j++)
            {
                // make sure we set the contact point here
                solver_data.contact_point = j;
                vec3 pos_fix              = GLM_VEC3_ZERO_INIT;
                gsk_physics_position_solver(solver_data, pos_fix);

                __integrate_position_add(entity_a, pos_fix);
                glm_vec3_negate(pos_fix);
                __integrate_position_add(entity_b, pos_fix);
            }
        }
    }

    /*==== Clear Solvers List ========================================*/

    while (p_solver->solvers_list->is_list_empty == FALSE)
    {
        gsk_physics_solver_pop();
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
s_physics_world_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init         = (gsk_ECSSubscriber)init,
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                            }));
}
//-----------------------------------------------------------------------------