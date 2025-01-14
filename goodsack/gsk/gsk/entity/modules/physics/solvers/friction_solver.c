/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "friction_solver.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"
#include "util/vec_colors.h"

#define DEFAULT_RESTITUION 0.2f

#define DEBUG_POINTS    0 // 0 -- OFF | value = entity id
#define ENABLE_ROTATION 0

static void
__calc_relative_velocity(_SolverData solver_data,
                         vec3 ra_perp,
                         vec3 rb_perp,
                         vec3 linear_velocity_a,
                         vec3 angular_velocity_a,
                         vec3 linear_velocity_b,
                         vec3 angular_velocity_b,
                         float *dest_relative_velocity)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker                = collision_result->physics_mark;
    gsk_DynamicBody body_a                = marker.body_a;
    gsk_DynamicBody body_b                = marker.body_b;

    // angular velocity
    vec3 angular_linear_velocity_a, angular_linear_velocity_b;
    glm_vec3_mul(ra_perp, angular_velocity_a, angular_linear_velocity_a);
    glm_vec3_mul(rb_perp, angular_velocity_b, angular_linear_velocity_b);

    // calculate relative velocity
    /* (a_vel + a_ang_vel) - (b_vel + b_ang_vel) - */

    vec3 cmba, cmbb;
    glm_vec3_add(linear_velocity_a, angular_linear_velocity_a, cmba);
    glm_vec3_add(linear_velocity_b, angular_linear_velocity_b, cmbb);
    glm_vec3_sub(cmba, cmbb, dest_relative_velocity);
}

void
impulse_solver_with_rotation_friction(_SolverData solver_data)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker                = collision_result->physics_mark;

    struct ComponentTransform *transform   = solver_data.p_transform;
    struct ComponentRigidbody *rigidbody_a = solver_data.p_rigidbody;

    gsk_DynamicBody body_a = marker.body_a;
    gsk_DynamicBody body_b = marker.body_b;

    vec3 ra, ra_perp;
    vec3 rb, rb_perp;

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    vec3 relative_velocity;

    float F  = 0;
    float Ft = 0;

    float restitution = DEFAULT_RESTITUION; // Bounce factor

    // store collision_normal
    glm_vec3_copy(collision_result->points.normal, collision_normal);
    // glm_vec3_negate(collision_normal);

    // calculate r-values + relative velocity
    {
        glm_vec3_sub(collision_result->points.point_a, body_a.position, ra);
        glm_vec3_cross(ra, collision_result->points.point_a, ra_perp);
        glm_vec3_normalize(ra_perp);

        glm_vec3_sub(collision_result->points.point_b, body_b.position, rb);
        glm_vec3_cross(collision_result->points.point_b, rb, rb_perp);
        glm_vec3_normalize(rb_perp);

        // calculate relative velocity
        __calc_relative_velocity(solver_data,
                                 ra_perp,
                                 rb_perp,
                                 body_a.linear_velocity,
                                 body_a.angular_velocity,
                                 body_b.linear_velocity,
                                 body_b.angular_velocity,
                                 relative_velocity);
    }

    // calculate F
    {
        float vDotN = (glm_vec3_dot(relative_velocity, collision_normal));

        f32 ra_perpDotN = glm_dot(ra_perp, collision_normal);
        f32 rb_perpDotN = glm_dot(rb_perp, collision_normal);

        f32 denom = body_a.inverse_mass + body_b.inverse_mass +
                    (pow(ra_perpDotN, 2) * body_a.inverse_inertia) +
                    (pow(rb_perpDotN, 2) * body_b.inverse_inertia);

        F = -(1.0f + restitution) * vDotN;
        F /= denom;
        // prevent negative impulse
        if (vDotN > 0.0f) F = 0;
    }

    // calculate impulse, torque
    vec3 impulse, torque = GLM_VEC3_ZERO_INIT;
    {
        glm_vec3_scale(collision_normal, F, impulse);

        vec3 torque;
        glm_vec3_cross(ra, impulse, torque);
        // glm_vec3_negate(torque);
        //  scale torque by inverse inertia
        glm_vec3_scale(torque, body_a.inverse_inertia, torque);

        // NOTE: May need to be done AFTER torque calculation
        // scale impulse by inverse mass
        glm_vec3_scale(impulse, body_a.inverse_mass, impulse);
    }

    // Debug some data
    {
        if (DEBUG_POINTS && solver_data.entity.id == DEBUG_POINTS)
        {

            gsk_debug_markers_push(
              solver_data.entity.ecs->renderer->debugContext,
              MARKER_RAY,
              solver_data.entity.id + 6,
              collision_result->points.point_a,
              rigidbody_a->linear_velocity,
              1,
              VCOL_BLUE,
              FALSE);

            gsk_debug_markers_push(
              solver_data.entity.ecs->renderer->debugContext,
              MARKER_RAY,
              solver_data.entity.id + 7,
              // solver_data.p_transform->position,
              collision_result->points.point_a,
              torque,
              100,
              VCOL_RED,
              FALSE);

            gsk_debug_markers_push(
              solver_data.entity.ecs->renderer->debugContext,
              MARKER_RAY,
              solver_data.entity.id + 11,
              // solver_data.p_transform->position,
              collision_result->points.point_a,
              ra_perp,
              1,
              VCOL_WHITE,
              FALSE);
            gsk_debug_markers_push(
              solver_data.entity.ecs->renderer->debugContext,
              MARKER_RAY,
              solver_data.entity.id + 20,
              solver_data.p_transform->position,
              ra,
              1,
              VCOL_GREEN,
              FALSE);
#if 0
            gsk_debug_markers_push(
              solver_data.entity.ecs->renderer->debugContext,
              MARKER_RAY,
              solver_data.entity.id + 12,
              // solver_data.p_transform->position,
              collision_result->points.point_a,
              ra_perp,
              10,
              VCOL_CYAN,
              FALSE);
#endif
        };
    }

    vec3 body_b_lin_vel, body_b_ang_vel;
    glm_vec3_copy(body_b.linear_velocity, body_b_lin_vel);
    glm_vec3_copy(body_b.angular_velocity, body_b_ang_vel);
    // apply impulses
    {
        glm_vec3_add(
          rigidbody_a->linear_velocity, impulse, rigidbody_a->linear_velocity);

#if (ENABLE_ROTATION)
        glm_vec3_add(
          rigidbody_a->angular_velocity, torque, rigidbody_a->angular_velocity);

        // TESTING
        glm_vec3_sub(body_b_lin_vel, impulse, body_b_lin_vel);
        glm_vec3_sub(body_b_ang_vel, torque, body_b_ang_vel);
#endif // (ENABLE_ROTATION)
    }

    // -----------------------------
    // Friction Step (re-calculate new impulse based on friction tangent)
    // -----------------------------

    // calculate relative velocity
    {
        __calc_relative_velocity(solver_data,
                                 ra_perp,
                                 rb_perp,
                                 rigidbody_a->linear_velocity,
                                 rigidbody_a->angular_velocity,
                                 body_b_lin_vel,
                                 body_b_ang_vel,
                                 relative_velocity);
    }

    // create tangent
    vec3 tangent = GLM_VEC3_ZERO_INIT;
    {
        // tangent = velocity - dot(velocity, normal) * normal;
        glm_vec3_scale(collision_normal,
                       glm_vec3_dot(relative_velocity, collision_normal),
                       tangent);
        glm_vec3_sub(relative_velocity, tangent, tangent);

        // check tangent for near-zero
        float zerodist = glm_vec3_distance(tangent, GLM_VEC3_ZERO);
        // LOG_INFO("%f ", zerodist);
        if (zerodist <= 0.005f) { return; }

        // proceed with calculation for tangent
        glm_vec3_normalize(tangent);
    }

    // calculate Ft
    {
        f32 vDotT       = (glm_vec3_dot(relative_velocity, tangent));
        f32 ra_perpDotT = glm_dot(ra_perp, tangent);
        f32 rb_perpDotT = glm_dot(rb_perp, tangent);

        f32 denom = body_a.inverse_mass + body_b.inverse_mass +
                    (pow(ra_perpDotT, 2) * body_a.inverse_inertia) +
                    (pow(rb_perpDotT, 2) * body_b.inverse_inertia);

        Ft = -vDotT;
        Ft /= denom;
    }

    // create friction_impulse, friction_torque
    vec3 friction_impulse, friction_torque = GLM_VEC3_ZERO_INIT;
    {
        f32 sf =
          (rigidbody_a->static_friction + rigidbody_a->static_friction) * 0.5f;
        f32 df =
          (rigidbody_a->dynamic_friction + rigidbody_a->dynamic_friction) *
          0.5f;

        if (fabs(Ft) <= F * sf)
        {
            glm_vec3_scale(tangent, Ft, friction_impulse);
        } else
        {
            glm_vec3_scale(tangent, -F * df, friction_impulse);
        }

        glm_vec3_cross(ra, friction_impulse, friction_torque);
        //  scale torque by inverse inertia
        glm_vec3_scale(
          friction_torque, body_a.inverse_inertia, friction_torque);

        // NOTE: May need to be done AFTER torque calculation
        // scale impulse by inverse mass
        glm_vec3_scale(friction_impulse, body_a.inverse_mass, friction_impulse);
    }

    // apply friction impulses
    {
        glm_vec3_add(rigidbody_a->linear_velocity,
                     friction_impulse,
                     rigidbody_a->linear_velocity);

#if (ENABLE_ROTATION)
        glm_vec3_add(rigidbody_a->angular_velocity,
                     friction_torque,
                     rigidbody_a->angular_velocity);
#endif // (ENABLE_ROTATION)
    }

    // --------------
    // DEBUG SOME LINES
    {
        if (DEBUG_POINTS && solver_data.entity.id == DEBUG_POINTS)
        {
            gsk_debug_markers_push(
              solver_data.entity.ecs->renderer->debugContext,
              MARKER_RAY,
              solver_data.entity.id + 8,
              solver_data.p_transform->position,
              relative_velocity,
              1,
              VCOL_CYAN,
              FALSE);

            gsk_debug_markers_push(
              solver_data.entity.ecs->renderer->debugContext,
              MARKER_RAY,
              solver_data.entity.id + 9,
              // solver_data.p_transform->position,
              collision_result->points.point_a,
              friction_torque,
              10,
              VCOL_ORANGE,
              FALSE);

            gsk_debug_markers_push(
              solver_data.entity.ecs->renderer->debugContext,
              MARKER_RAY,
              solver_data.entity.id + 10,
              // solver_data.p_transform->position,
              collision_result->points.point_a,
              friction_impulse,
              10,
              VCOL_PURPLE,
              FALSE);
        }
    }
}
