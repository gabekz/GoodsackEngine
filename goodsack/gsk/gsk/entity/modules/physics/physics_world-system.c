/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_world-system.h"

#include "entity/ecs.h"
#include "entity/ecsdefs.h"

#include "physics/physics_solver.h"
#include "physics/physics_types.h"

#include "entity/modules/physics/physics_util.h"
#include "entity/modules/physics/solvers/friction_solver.h"
#include "entity/modules/physics/solvers/position_solver.h"
#include "entity/modules/physics/solvers/solver_data.h"

#include "entity/modules/physics/solvers/joint_solver.h"

#include "entity/modules/transform/transform.h"

#include "physics/physics_sat.h"

#include "core/device/device.h"

#include "util/maths.h"
#include "util/sysdefs.h"
#include "util/vec_colors.h"

#include "runtime/gsk_runtime_debug.h"

#define _APPLY_GRAVITY_FIRST TRUE

#define _VELOCITY_ITERATIONS 6 // 6
#define _FRICTION_ITERATIONS 2 // 2
#define _POSITION_ITERATIONS 1 // 1

#define _UPDATE_DB_MODE      2
#define _NEW_POSITION_SOLVER 1

#define _POSITION_UPDATES_ROTATION 1

#define _USING_CONSTRAINTS 0
#define A_ANCHOR           1.0f
#define B_ANCHOR           0.5f
#define A_ID               303
#define B_ID               302
#define _SOFTNESS          0.1f
#define _REST_LEN          0.2f
#define _BETA              0.2f // higher for non-rb

#if 0
static f32
__calc_effective_mass(gsk_PhysicsSolverData solver_data,
                      vec3 point_a,
                      vec3 point_b)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker                = collision_result->physics_mark;

    gsk_CollisionPoints points =
      collision_result->manifold.contacts[solver_data.contact_point];

    gsk_DynamicBody body_a = marker.body_a;
    gsk_DynamicBody body_b = marker.body_b;

    vec3 ra, rb;
    glm_vec3_sub(point_a, body_a.position, ra);
    glm_vec3_sub(point_b, body_b.position, rb);

    vec3 raxn, rbxn;
    glm_vec3_cross(ra, collision_result->manifold.normal, raxn);
    glm_vec3_cross(rb, collision_result->manifold.normal, rbxn);

    // TODO: probably want this?
#if 0
    return body_a.inverse_mass + body_b.inverse_mass +
           (glm_vec3_norm(raxn) * body_a.inverse_inertia) +
           (glm_vec3_norm(rbxn) * body_b.inverse_inertia);
#else
    return body_a.inverse_mass + body_b.inverse_mass;
#endif
}
#endif

#if 0
static void
__calc_relative_velocity(gsk_PhysicsSolverData solver_data,
                         vec3 out_relative_velocity,
                         vec3 out_ra,
                         vec3 out_rb)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker                = collision_result->physics_mark;
    gsk_CollisionPoints points =
      collision_result->manifold.contacts[solver_data.contact_point];

    gsk_DynamicBody body_a = marker.body_a;
    gsk_DynamicBody body_b = marker.body_b;

    vec3 ra, ra_perp;
    vec3 rb, rb_perp;

    // calculate r-values + relative velocity
    {
        glm_vec3_sub(points.point_a, body_a.position, ra);
        glm_vec3_cross(body_a.angular_velocity, ra, ra_perp);

        glm_vec3_sub(points.point_b, body_b.position, rb);
        glm_vec3_cross(body_b.angular_velocity, rb, rb_perp);

        vec3 cmba, cmbb;
        glm_vec3_add(body_a.linear_velocity, ra_perp, cmba);
        glm_vec3_add(body_b.linear_velocity, rb_perp, cmbb);
        glm_vec3_sub(cmba, cmbb, out_relative_velocity);

        glm_vec3_copy(ra, out_ra);
        glm_vec3_copy(rb, out_rb);
    }
}
#endif

//-----------------------------------------------------------------------------
static void
__apply_gravity(gsk_Entity entity, f64 delta)
{
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return; }

    // --
    // -- Add gravity to net force (mass considered)

    // mass * gravity
    vec3 mG = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(cmp_rigidbody->gravity, delta, mG);

    // NOTE: adding to linear_velocity rather than force_velocity seems to cause
    // a bit of a jitter on obb_obb
    glm_vec3_add(
      cmp_rigidbody->force_velocity, mG, cmp_rigidbody->force_velocity);
}
//-----------------------------------------------------------------------------

#if 0
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
    glm_vec3_sub(point, cmp_transform->position, ra);
    glm_vec3_cross(ra, impulse, new_torque);

#if 0
    glm_vec3_scale(new_torque, cmp_rigidbody->inverse_inertia, new_torque);

    // add new torque
    glm_vec3_add(cmp_rigidbody->angular_velocity,
                 new_torque,
                 cmp_rigidbody->angular_velocity);
#else
    mat3 rot = GLM_MAT3_ZERO_INIT;
    glm_quat_mat3(cmp_transform->rotation, rot);

    mat3 inertia_world = GLM_MAT3_ZERO_INIT;
    gsk_physics_util_inverse_inertia_world(
      rot, cmp_rigidbody->inverse_inertia_tensor, inertia_world);
    // angular velocity += I^-1_world * angular_impulse
    vec3 dw;
    glm_mat3_mulv(inertia_world, new_torque, dw);
    glm_vec3_add(
      cmp_rigidbody->angular_velocity, dw, cmp_rigidbody->angular_velocity);
#endif
}
#else
static void
__apply_impulse_at_point(gsk_Entity entity, vec3 impulse, vec3 point)
{
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return; }

    /*
     * Linear velocity:
     * v += impulse * inv_mass
     */
    vec3 dv = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(impulse, cmp_rigidbody->inverse_mass, dv);
    glm_vec3_add(
      cmp_rigidbody->linear_velocity, dv, cmp_rigidbody->linear_velocity);

    if (cmp_rigidbody->disable_rotation == TRUE) { return; }

    /*
     * IMPORTANT:
     * Torque arm must be point - center_of_mass, not point -
     * transform.position.
     */
    gsk_DynamicBody body = gsk_physics_util_create_dynamic_body(entity);

    vec3 ra = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(point, body.center_of_mass, ra);

    vec3 angular_impulse = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(ra, impulse, angular_impulse);

    /*
     * angular_velocity += inverse_inertia_world * angular_impulse
     */
    vec3 dw = GLM_VEC3_ZERO_INIT;
    glm_mat3_mulv(body.inertia_tensor, angular_impulse, dw);

    glm_vec3_add(
      cmp_rigidbody->angular_velocity, dw, cmp_rigidbody->angular_velocity);
}
#endif

#if 0
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

    vec3 old_pos = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(cmp_transform->position, old_pos);

    // add new impulse
    glm_vec3_add(cmp_transform->position, new_impulse, cmp_transform->position);

#if _POSITION_UPDATES_ROTATION
    // calculate torque

    if (cmp_rigidbody->disable_rotation == TRUE) { return; }

    vec3 new_torque = GLM_VEC3_ZERO_INIT;
    vec3 ra         = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(point, old_pos, ra);
    glm_vec3_cross(ra, impulse, new_torque);

#if 0
    glm_vec3_scale(new_torque, cmp_rigidbody->inverse_inertia, new_torque);

    // transform_rotate(cmp_transform, new_torque);

    vec3 angularDeg = GLM_VEC3_ZERO_INIT;
    angularDeg[0]   = glm_deg(new_torque[0]);
    angularDeg[1]   = glm_deg(new_torque[1]);
    angularDeg[2]   = glm_deg(new_torque[2]);

    vec3 test = {angularDeg[0], angularDeg[1], angularDeg[2]};
#endif

    mat3 rot = GLM_MAT3_ZERO_INIT;
    glm_quat_mat3(cmp_transform->rotation, rot);

    mat3 inertia_world = GLM_MAT3_ZERO_INIT;
    gsk_physics_util_inverse_inertia_world(
      rot, cmp_rigidbody->inverse_inertia_tensor, inertia_world);
    // angular velocity += I^-1_world * angular_impulse
    vec3 dw;
    glm_mat3_mulv(inertia_world, new_torque, dw);
    transform_rotate(cmp_transform, dw);
#endif // _POSITION_UPDATES_ROTATION
}
#else
static void
__apply_impulse_at_point_position(gsk_Entity entity, vec3 impulse, vec3 point)
{
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }

    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    if (cmp_rigidbody->is_kinematic == TRUE) { return; }

    gsk_C_Transform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    /*
     * Snapshot current body state before modifying transform.
     */
    gsk_DynamicBody body = gsk_physics_util_create_dynamic_body(entity);

    /*
     * Linear position correction.
     *
     * This moves the model origin by the same delta that the COM should move.
     * That is okay for pure translation.
     */
    vec3 dp = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(impulse, cmp_rigidbody->inverse_mass, dp);
    glm_vec3_add(cmp_transform->position, dp, cmp_transform->position);

#if _POSITION_UPDATES_ROTATION
    if (cmp_rigidbody->disable_rotation == TRUE) { return; }

    /*
     * Angular correction around COM.
     */
    vec3 ra = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(point, body.center_of_mass, ra);

    vec3 angular_impulse = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(ra, impulse, angular_impulse);

    vec3 dtheta = GLM_VEC3_ZERO_INIT;
    glm_mat3_mulv(body.inertia_tensor, angular_impulse, dtheta);

    transform_rotate(cmp_transform, dtheta);

    glm_vec3_copy(body.center_of_mass, cmp_rigidbody->center_of_mass);
#endif
}
#endif

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
    // if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }

    // gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    // if (cmp_rigidbody->is_kinematic == TRUE) { return; }

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

#if !(_UPDATE_DB_MODE)
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
#endif
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

#if _USING_CONSTRAINTS
    /*---- PUSH TEST CONSTRAINT --------------------------------------*/
    {
        gsk_ConstraintResult constraint = {
          .ent_a_id       = A_ID,
          .ent_b_id       = B_ID,
          .local_anchor_a = {0, A_ANCHOR, 0},
          .local_anchor_b = {0, B_ANCHOR, 0},
          .softness       = 2.0f,
          .rest_distance  = _REST_LEN,
        };

        gsk_physics_solver_push_constraint(constraint);
    }
    {
        gsk_ConstraintResult constraint = {
          .ent_a_id       = B_ID,
          .ent_b_id       = 304,
          .local_anchor_a = {0, -B_ANCHOR, 0},
          .local_anchor_b = {0, 0.2f, 0},
          .softness       = 2.0f,
          .rest_distance  = 0.2f,
        };

        gsk_physics_solver_push_constraint(constraint);
    }
#endif _USING_CONSTRAINTS

    u8 total_solvers     = p_solver->solvers_list->list_next;
    u8 total_constraints = p_solver->constraints_list->list_next;

#if _APPLY_GRAVITY_FIRST
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
            gsk_CollisionResult *pResult =
              LIST_GET(p_solver->solvers_list, i_solver);

            // --
            // construct solver_data used to pass into solver functions
            gsk_PhysicsSolverData solver_data = {
              .p_collision_result = pResult,
              .delta              = delta,
              .contact_point      = 0,
              .entity             = entity,
            };

            if (solver_data.p_collision_result->is_trigger_response == TRUE)
            {
                continue;
            }

            // TODO: we are probably going to need to reconstruct the body_a and
            // body_b parts for each iteration because of joints (they might
            // need to see updated impulses)

            gsk_Entity entity_a =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_a_id);
            gsk_Entity entity_b =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_b_id);

#if _UPDATE_DB_MODE == 1
            solver_data.p_collision_result->physics_mark =
              gsk_physics_util_create_physics_mark(entity_a, entity_b);
#endif

            for (int j = 0; j < pResult->manifold.contacts_count; j++)
            {
#if _UPDATE_DB_MODE == 2
                solver_data.p_collision_result->physics_mark =
                  gsk_physics_util_create_physics_mark(entity_a, entity_b);
#endif
                solver_data.contact_point = j;

                // run the solver
                gsk_PhysicsSolverLambda output = gsk_physics_impulse_solver(
                  solver_data, pResult->manifold.contacts[j].lambda_n);

                _apply_solver_to_bodies(entity_a, entity_b, output, j, pResult);
            }
        }

        /*---- constraint contact velocity -------------------------------*/
        for (int i_constraint = 0; i_constraint < total_constraints;
             i_constraint++)
        {
#if 1
            gsk_ConstraintResult *p_constraint =
              LIST_GET(p_solver->constraints_list, i_constraint);

            gsk_Entity ent_a = gsk_ecs_ent(entity.ecs, p_constraint->ent_a_id);
            gsk_Entity ent_b = gsk_ecs_ent(entity.ecs, p_constraint->ent_b_id);

            vec3 world_a, world_b;
            {
                gsk_C_Transform *cmp_transform =
                  gsk_ecs_get(ent_a, C_TRANSFORM);

                transform_point_local_to_world(
                  cmp_transform, p_constraint->local_anchor_a, world_a);
            }
            {
                gsk_C_Transform *cmp_transform =
                  gsk_ecs_get(ent_b, C_TRANSFORM);

                transform_point_local_to_world(
                  cmp_transform, p_constraint->local_anchor_b, world_b);
            }

            gsk_ConstraintSolverData solver_data = {
              .delta    = 0.05f,
              .beta     = _BETA,
              .softness = p_constraint->softness,
              .physics_mark =
                gsk_physics_util_create_physics_mark(ent_a, ent_b),
            };
            glm_vec3_copy(world_a, solver_data.world_a);
            glm_vec3_copy(world_b, solver_data.world_b);

            gsk_ConstraintSolverOutput output =
              gsk_physics_joint_velocity_solver(solver_data);

            for (int axis = 0; axis < 3; axis++)
            {
                vec3 impulse = GLM_VEC3_ZERO_INIT;
                glm_vec3_negate_to(output.impulse_axes[axis], impulse);
                // glm_vec3_copy(output.impulse_axes[axis], impulse);
                __apply_impulse_at_point(ent_a, impulse, world_a);
                glm_vec3_negate(impulse);
                __apply_impulse_at_point(ent_b, impulse, world_b);

#if 0
                GSK_DEBUG_DRAW_RAY(
                  world_a, impulse, glm_vec3_norm2(impulse) * 5.0f, VCOL_CYAN);
#endif
            }
#endif
        }
    }

#if 1

    /*==== FRICTION Impulse Solver ===================================*/

    for (int iter = 0; iter < _FRICTION_ITERATIONS; ++iter)
    {
        for (int i_solver = 0; i_solver < total_solvers; ++i_solver)
        {
            gsk_CollisionResult *pResult =
              LIST_GET(p_solver->solvers_list, i_solver);

            // --
            // construct solver_data used to pass into solver functions
            gsk_PhysicsSolverData solver_data = {
              .p_collision_result = pResult,
              .delta              = delta,
              .contact_point      = 0,
              .entity             = entity,
            };

            if (solver_data.p_collision_result->is_trigger_response == TRUE)
            {
                continue;
            }

            gsk_Entity entity_a =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_a_id);
            gsk_Entity entity_b =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_b_id);

#if _UPDATE_DB_MODE == 1
            solver_data.p_collision_result->physics_mark =
              gsk_physics_util_create_physics_mark(entity_a, entity_b);
#endif
            for (int j = 0; j < pResult->manifold.contacts_count; j++)
            {
#if _UPDATE_DB_MODE == 2
                solver_data.p_collision_result->physics_mark =
                  gsk_physics_util_create_physics_mark(entity_a, entity_b);
#endif
                solver_data.contact_point = j;

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
            gsk_CollisionResult *pResult =
              LIST_GET(p_solver->solvers_list, i_solver);

            // --
            // construct solver_data used to pass into solver functions
            gsk_PhysicsSolverData solver_data = {
              .p_collision_result = pResult,
              .delta              = delta,
              .contact_point      = 0,
              .entity             = entity,
            };

            if (solver_data.p_collision_result->is_trigger_response == TRUE)
            {
                continue;
            }

            gsk_Entity entity_a =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_a_id);
            gsk_Entity entity_b =
              gsk_ecs_ent(entity.ecs, solver_data.p_collision_result->ent_b_id);

#if _UPDATE_DB_MODE == 1
            solver_data.p_collision_result->physics_mark =
              gsk_physics_util_create_physics_mark(entity_a, entity_b);
#endif //_UPDATE_DB_MODE
            for (int j = 0; j < pResult->manifold.contacts_count; j++)
            {
                solver_data.contact_point = j;

#if _UPDATE_DB_MODE == 2
                solver_data.p_collision_result->physics_mark =
                  gsk_physics_util_create_physics_mark(entity_a, entity_b);
#endif //_UPDATE_DB_MODE

#if !(_NEW_POSITION_SOLVER)
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

                    // NOTE: the way we are getting the rotation here is
                    // somehow wrong.
                    transform_point_local_to_world(
                      cmp_transform,
                      pResult->manifold.contacts[j].local_point_a,
                      world_a);
                }
                {
                    gsk_C_Transform *cmp_transform =
                      gsk_ecs_get(entity_b, C_TRANSFORM);

                    transform_point_local_to_world(
                      cmp_transform,
                      pResult->manifold.contacts[j].local_point_b,
                      world_b);
                }

                vec3 ab                = GLM_VEC3_ZERO_INIT;
                f32 contact_separation = 0.0f;
                {
                    glm_vec3_sub(world_b, world_a, ab);
                    contact_separation = glm_vec3_norm(ab);
                }

                // good settings?
                // f32 steering       = 0.20f;
                // f32 max_correction = 0.0005f;
                // f32 slop           = 0.0001f;

                f32 depth          = contact_separation;
                f32 steering       = 0.20f;
                f32 max_correction = 0.0005f;
                f32 slop           = 0.0002f;

                // f32 force = steering * (depth + slop);
                f32 force = steering * (depth + slop);
                // CLAMP(force, -max_correction, 0.0f);

                gsk_DynamicBody *body_a = &pResult->physics_mark.body_a;
                gsk_DynamicBody *body_b = &pResult->physics_mark.body_b;

                gsk_CollisionPoints points =
                  pResult->manifold.contacts[solver_data.contact_point];

                vec3 impulse = GLM_VEC3_ZERO_INIT;
#if 1
                vec3 rvel, ra, rb;
                // TODO: might want to check against points.point_a/b
                gsk_physics_util_relative_velocity(
                  *body_a, *body_b, world_a, world_b, rvel, ra, rb);

                glm_vec3_normalize_to(ab, impulse);

                f32 mass = gsk_physics_util_effective_mass_axis(
                  body_a, body_b, ra, rb, ab);
#else
                f32 mass = __calc_effective_mass(solver_data, world_a, world_b);
#endif

                force = FDIV_SAFE(force, mass);

#if 0
                glm_vec3_scale(
                  solver_data.p_collision_result->manifold.contacts[j].normal,
                  force,
                  impulse);
#else
                // glm_vec3_normalize_to(ab, impulse);
                glm_vec3_scale(impulse, force, impulse);
#endif

                const f32 debug_ray_scale = sqrt(force * 10);

                // GSK_DEBUG_DRAW_POINT(body_a->position, 1.0f, VCOL_GREEN);
                GSK_DEBUG_DRAW_POINT(world_a, 1.0f, VCOL_GREEN);
                GSK_DEBUG_DRAW_RAY(
                  world_a, impulse, debug_ray_scale, VCOL_GREEN);

                __apply_impulse_at_point_position(entity_a, impulse, world_a);
                glm_vec3_negate(impulse);
                __apply_impulse_at_point_position(entity_b, impulse, world_b);

                // GSK_DEBUG_DRAW_POINT(body_b->position, 1.0f, VCOL_RED);
                GSK_DEBUG_DRAW_POINT(world_b, 1.0f, VCOL_RED);
                GSK_DEBUG_DRAW_RAY(world_b, impulse, debug_ray_scale, VCOL_RED);
#endif
            }
        }

        // JOINT POSITION SOLVER //

#if 0
        for (int i_joint = 0; i_joint < total_joints; ++i_joint)
        {
            // apply joint position solver here
            // please

            


            gsk_physics_distance_joint_position_solver()
        }
#endif

#if 1
        for (int i_constraint = 0; i_constraint < total_constraints;
             i_constraint++)
        {
            gsk_ConstraintResult *p_constraint =
              LIST_GET(p_solver->constraints_list, i_constraint);

            gsk_Entity ent_a = gsk_ecs_ent(entity.ecs, p_constraint->ent_a_id);
            gsk_Entity ent_b = gsk_ecs_ent(entity.ecs, p_constraint->ent_b_id);

            gsk_PhysicsMark mark =
              gsk_physics_util_create_physics_mark(ent_a, ent_b);

            vec3 world_a, world_b;
            {
                gsk_C_Transform *cmp_transform =
                  gsk_ecs_get(ent_a, C_TRANSFORM);

                transform_point_local_to_world(
                  cmp_transform, p_constraint->local_anchor_a, world_a);
            }
            {
                gsk_C_Transform *cmp_transform =
                  gsk_ecs_get(ent_b, C_TRANSFORM);

                transform_point_local_to_world(
                  cmp_transform, p_constraint->local_anchor_b, world_b);
            }

            vec3 impulse = GLM_VEC3_ZERO_INIT;
            gsk_physics_distance_joint_position_solver(
              world_a,
              world_b,
              mark.body_a.inverse_mass,
              mark.body_b.inverse_mass,
              p_constraint->rest_distance,
              impulse);

            __apply_impulse_at_point_position(ent_b, impulse, world_b);
            glm_vec3_negate(impulse);
            __apply_impulse_at_point_position(ent_a, impulse, world_a);

            // NOTE: change this
            continue;

            vec3 impulse_a, impulse_b;
            glm_vec3_negate(impulse);
            glm_vec3_scale(impulse, mark.body_a.inverse_mass, impulse_a);
            __integrate_position_add(ent_a, impulse_a);

// glm_vec3_scale(impulse, mark.body_a.inverse_mass, impulse_b);
#if 0
        glm_vec3_negate_to(impulse_a, impulse_b);
#else
            glm_vec3_negate(impulse);
            glm_vec3_scale(impulse, mark.body_b.inverse_mass, impulse_b);
#endif
            __integrate_position_add(ent_b, impulse_b);

#endif
        }
    }

#if 1
    /*==== Clear Solvers List ========================================*/

    gsk_physics_solver_clear_constraints();
#endif

#if !(_APPLY_GRAVITY_FIRST)
    /*==== Apply Gravity ===================================*/

    for (int i = 0; i < entity.ecs->nextIndex; i++)
    {
        gsk_Entity ent =
          gsk_ecs_ent(entity.ecs, (gsk_EntityId)entity.ecs->p_ent_ids[i]);

        __apply_gravity(ent, delta);
    }
#endif
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
static void
late_update(gsk_Entity entity)
{
    // NOTE: hack so this only runs on one entity.
    // should have proper support for singleton ECS systems.
    if (entity.index != 0) { return; }

    /*==== Clear Solvers List ========================================*/

    gsk_physics_solver_clear();
}

//-----------------------------------------------------------------------------
void
s_physics_world_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init         = (gsk_ECSSubscriber)init,
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                              .late_update  = (gsk_ECSSubscriber)late_update,
                            }));
}
//-----------------------------------------------------------------------------