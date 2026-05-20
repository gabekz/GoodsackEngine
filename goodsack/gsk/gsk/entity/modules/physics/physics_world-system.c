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

#include "physics/physics_sat.h"

#include "core/device/device.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#define _VELOCITY_ITERATIONS 6
#define _FRICTION_ITERATIONS 3
#define _POSITION_ITERATIONS 1

static void
_obb_local_to_world(vec3 center, mat3 rot, vec3 local, vec3 out)
{
    glm_vec3_copy(center, out);

    for (int i = 0; i < 3; ++i)
    {
        vec3 term = GLM_VEC3_ZERO_INIT;
        glm_vec3_scale(rot[i], local[i], term);
        glm_vec3_add(out, term, out);
    }
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

static void
__apply_impulse_at_point(gsk_Entity entity, vec3 impulse, vec3 point)
{
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return; }

    vec3 new_impulse = GLM_VEC3_ZERO_INIT;

    glm_vec3_scale(impulse, cmp_rigidbody->inverse_mass, new_impulse);

    // add new impulse
    glm_vec3_add(cmp_rigidbody->linear_velocity,
                 new_impulse,
                 cmp_rigidbody->linear_velocity);

    // calculate torque

    if (cmp_rigidbody->disable_rotation == TRUE) { return; }

    gsk_C_Transform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    vec3 new_torque = GLM_VEC3_ZERO_INIT;
    vec3 ra         = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(point, cmp_transform->world_position, ra);
    glm_vec3_cross(ra, impulse, new_torque);
    glm_vec3_scale(new_torque, cmp_rigidbody->inverse_inertia, new_torque);

    // add new torque
    glm_vec3_add(cmp_rigidbody->angular_velocity,
                 new_torque,
                 cmp_rigidbody->angular_velocity);
}

static void
__apply_impulse_at_point_position(gsk_Entity entity, vec3 impulse, vec3 point)
{
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return; }

    gsk_C_Transform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    vec3 new_impulse = GLM_VEC3_ZERO_INIT;

    glm_vec3_scale(impulse, cmp_rigidbody->inverse_mass, new_impulse);

    // add new impulse
    glm_vec3_add(cmp_transform->position, new_impulse, cmp_transform->position);

#if 0
    // calculate torque

    if (cmp_rigidbody->disable_rotation == TRUE) { return; }

    gsk_C_Transform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    vec3 new_torque = GLM_VEC3_ZERO_INIT;
    vec3 ra         = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(point, cmp_transform->world_position, ra);
    glm_vec3_cross(ra, impulse, new_torque);
    glm_vec3_scale(new_torque, cmp_rigidbody->inverse_inertia, new_torque);

    // add new torque
    glm_vec3_add(cmp_rigidbody->angular_velocity,
                 new_torque,
                 cmp_rigidbody->angular_velocity);
#endif
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
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
_apply_solver_to_bodies(gsk_Entity entity_a,
                        gsk_Entity entity_b,
                        gsk_PhysicsSolverLambda output,
                        u32 contact_index,
                        gsk_CollisionResult *p_result)
{
    // apply delta
    p_result->manifold.contacts[contact_index].lambda_t = output.lambda_t;
    p_result->manifold.contacts[contact_index].lambda_n = output.lambda_n;

    __apply_impulse_at_point(
      entity_a,
      output.impulse_total,
      p_result->manifold.contacts[contact_index].point_a);

    vec3 b_impulse = GLM_VEC3_ZERO_INIT;
    glm_vec3_negate_to(output.impulse_total, b_impulse);

    __apply_impulse_at_point(
      entity_b, b_impulse, p_result->manifold.contacts[contact_index].point_b);

    if (gsk_ecs_has(entity_a, C_RIGIDBODY))
    {
        gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity_a, C_RIGIDBODY);

        glm_vec3_copy(cmp_rigidbody->linear_velocity,
                      p_result->physics_mark.body_a.linear_velocity);
        glm_vec3_copy(cmp_rigidbody->angular_velocity,
                      p_result->physics_mark.body_a.angular_velocity);
    }

    if (gsk_ecs_has(entity_b, C_RIGIDBODY))
    {
        gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity_b, C_RIGIDBODY);

        glm_vec3_copy(cmp_rigidbody->linear_velocity,
                      p_result->physics_mark.body_b.linear_velocity);
        glm_vec3_copy(cmp_rigidbody->angular_velocity,
                      p_result->physics_mark.body_b.angular_velocity);
    }
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

#if 1
    /*==== Apply Gravity ===================================*/

    for (int i = 0; i < entity.ecs->nextIndex; i++)
    {
        gsk_Entity ent =
          gsk_ecs_ent(entity.ecs, (gsk_EntityId)entity.ecs->p_ent_ids[i]);

        __apply_gravity(ent, delta);
    }
#endif

    /*==== Velocity Impulse Solver ===================================*/

    for (int iter = 0; iter < _VELOCITY_ITERATIONS; ++iter)
    {
        /*---- collision contact velocity --------------------------------*/

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

                _apply_solver_to_bodies(entity_a, entity_b, output, j, pResult);
            }
        }

        /*---- constraint contact velocity -------------------------------*/
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

                _apply_solver_to_bodies(entity_a, entity_b, output, j, pResult);
            }
        }
    }
#endif

    /*==== Integrate velocity ========================================*/

    for (int i = 0; i < entity.ecs->nextIndex; i++)
    {
        gsk_Entity ent =
          gsk_ecs_ent(entity.ecs, (gsk_EntityId)entity.ecs->p_ent_ids[i]);

        //__apply_gravity(ent, delta);
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
                solver_data.contact_point = j;
#if 1
                // make sure we set the contact point here
                vec3 pos_fix = GLM_VEC3_ZERO_INIT;
                gsk_physics_position_solver(solver_data, pos_fix);

                __integrate_position_add(entity_a, pos_fix);
                glm_vec3_negate(pos_fix);
                __integrate_position_add(entity_b, pos_fix);
#else

                // need to get updated contact separation

                // glm_vec3_sub(pResult->manifold.contacts[j].point_a,
                // pResult->manifold.contacts[j].point_b)

                vec3 world_a = GLM_VEC3_ZERO_INIT, world_b = GLM_VEC3_ZERO_INIT;
                {
                    gsk_C_Transform *cmp_transform =
                      gsk_ecs_get(entity_a, C_TRANSFORM);

                    mat3 rot = GLM_MAT3_IDENTITY_INIT;
                    glm_mat4_pick3(cmp_transform->m4_rotation, rot);

                    _obb_local_to_world(
                      cmp_transform->world_position,
                      rot,
                      pResult->manifold.contacts[j].local_point_a,
                      world_a);
                }
                {
                    gsk_C_Transform *cmp_transform =
                      gsk_ecs_get(entity_b, C_TRANSFORM);

                    mat3 rot = GLM_MAT3_IDENTITY_INIT;
                    glm_mat4_pick3(cmp_transform->m4_rotation, rot);

                    _obb_local_to_world(
                      cmp_transform->world_position,
                      rot,
                      pResult->manifold.contacts[j].local_point_b,
                      world_b);
                }

                f32 contact_separation = 0.0f;
                {
                    vec3 ab = GLM_VEC3_ZERO_INIT;
                    glm_vec3_sub(world_b, world_a, ab);
                    contact_separation =
                      glm_vec3_dot(ab, pResult->manifold.contacts[j].normal);
                }

                f32 depth          = pResult->manifold.contacts[j].depth;
                f32 steering       = 0.2f;
                f32 max_correction = -0.2f;
                f32 slop           = 0.05f;

                f32 force = steering * (depth + slop);
                CLAMP(force, max_correction, 0.0f);

                vec3 impulse = GLM_VEC3_ZERO_INIT;
                glm_vec3_scale(pResult->manifold.contacts[j].normal,
                               -steering / 1.81f,
                               impulse);

                glm_vec3_negate(impulse);
                __apply_impulse_at_point_position(entity_a, impulse, impulse);
                glm_vec3_negate(impulse);
                __apply_impulse_at_point_position(entity_b, impulse, impulse);
#endif
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