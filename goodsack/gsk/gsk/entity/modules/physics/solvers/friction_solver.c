/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "friction_solver.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"
#include "util/vec_colors.h"

#include "entity/ecs.h"
#include "runtime/gsk_runtime_wrapper.h"

#define DEFAULT_RESTITUION 0.0f
#define ROLLING_FRICTION   0.01f

#define DEBUG_POINTS       0 // 0 -- OFF | value = entity id
#define CALCULATE_ROTATION TRUE

static void
__calc_relative_velocity(gsk_PhysicsSolverData solver_data,
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
    glm_vec3_copy(ra_perp, angular_linear_velocity_a);
    glm_vec3_copy(rb_perp, angular_linear_velocity_b);

    // calculate relative velocity
    /* (a_vel + a_ang_vel) - (b_vel + b_ang_vel) - */

    vec3 cmba, cmbb;
    glm_vec3_add(linear_velocity_a, angular_linear_velocity_a, cmba);
    glm_vec3_add(linear_velocity_b, angular_linear_velocity_b, cmbb);
    glm_vec3_sub(cmba, cmbb, dest_relative_velocity);
}

gsk_PhysicsSolverLambda
gsk_physics_impulse_solver(gsk_PhysicsSolverData solver_data, f32 lambda_n)
{
    gsk_PhysicsSolverLambda ret = {0};

    gsk_DebugContext *p_debug_context =
      solver_data.entity.ecs->renderer->debugContext;

    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker                = collision_result->physics_mark;
    gsk_CollisionPoints points =
      collision_result->manifold.contacts[solver_data.contact_point];

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
    glm_vec3_copy(points.normal, collision_normal);
    // glm_vec3_negate(collision_normal);

    // calculate r-values + relative velocity
    {
        glm_vec3_sub(points.point_a, body_a.position, ra);
        glm_vec3_cross(body_a.angular_velocity, ra, ra_perp);
        // glm_vec3_negate(ra_perp);
        // glm_vec3_normalize(ra_perp);

        glm_vec3_sub(points.point_b, body_b.position, rb);
        glm_vec3_cross(body_b.angular_velocity, rb, rb_perp);
        // glm_vec3_negate(rb_perp);
        // glm_vec3_normalize(rb_perp);

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
        vec3 raxn, rbxn;
        glm_vec3_cross(ra, collision_normal, raxn);
        glm_vec3_cross(rb, collision_normal, rbxn);

        float vDotN = (glm_vec3_dot(relative_velocity, collision_normal));

#if 0
        f32 ra_perpDotN = glm_dot(ra_perp, collision_normal);
        f32 rb_perpDotN = glm_dot(rb_perp, collision_normal);

        f32 denom = body_a.inverse_mass + body_b.inverse_mass +
                    (pow(ra_perpDotN, 2) * body_a.inverse_inertia) +
                    (pow(rb_perpDotN, 2) * body_b.inverse_inertia);
#else
        // NOTE: denom = contact's effective-mass
        f32 denom = body_a.inverse_mass + body_b.inverse_mass +
                    (glm_vec3_dot(raxn, raxn) * body_a.inverse_inertia) +
                    (glm_vec3_dot(rbxn, rbxn) * body_b.inverse_inertia);
#endif

        F = -(1.0f + restitution) * vDotN;
        F = (denom != 0) ? F / denom : F;

// prevent negative impulse
#if 0
        if (vDotN > 0.0f) F = 0;
#else
        f32 delta_lambda = F;
        f32 old_lambda   = lambda_n /* F */;
        ret.lambda_n     = fmaxf(old_lambda + delta_lambda, 0.0f);

        F = ret.lambda_n - old_lambda;
#endif
    }

    // calculate impulse, torque
    vec3 impulse, torque = GLM_VEC3_ZERO_INIT;
    {
        glm_vec3_scale(collision_normal, F, impulse);

        glm_vec3_cross(ra, impulse, torque);
        // glm_vec3_negate(torque);
        //  scale torque by inverse inertia
        // glm_vec3_scale(torque, body_a.inverse_inertia, torque);

        // NOTE: May need to be done AFTER torque calculation
        // scale impulse by inverse mass
        // glm_vec3_scale(impulse, body_a.inverse_mass, impulse);
    }

    // Debug some data
    {
        if (p_debug_context->physics_options.draw_friction &&
            solver_data.entity.id == gsk_runtime_get_debug_entity_id())
        {
            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 6,
                                   points.point_a,
                                   rigidbody_a->linear_velocity,
                                   1,
                                   VCOL_BLUE,
                                   FALSE);

            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 7,
                                   points.point_a,
                                   torque,
                                   1,
                                   VCOL_RED,
                                   FALSE);

            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 11,
                                   points.point_a,
                                   ra_perp,
                                   1,
                                   VCOL_WHITE,
                                   FALSE);
            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 12,
                                   points.point_a,
                                   rigidbody_a->angular_velocity,
                                   1,
                                   VCOL_YELLOW,
                                   FALSE);
            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 20,
                                   solver_data.p_transform->position,
                                   ra,
                                   1,
                                   VCOL_GREEN,
                                   FALSE);

            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 28,
                                   solver_data.p_transform->position,
                                   relative_velocity,
                                   1,
                                   VCOL_CYAN,
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

#if 1
    // apply impulses
    {
        vec3 impulse_a, impulse_b;
        vec3 torque_a, torque_b;
        glm_vec3_scale(impulse, body_a.inverse_mass, impulse_a);
        glm_vec3_scale(impulse, body_b.inverse_mass, impulse_b);

        glm_vec3_scale(torque, body_a.inverse_inertia, torque_a);
        glm_vec3_scale(torque, body_b.inverse_inertia, torque_b);

        glm_vec3_add(ret.impulse_a, impulse_a, ret.impulse_a);
        glm_vec3_add(ret.torque_a, torque_a, ret.torque_a);

        glm_vec3_sub(ret.impulse_b, impulse_b, ret.impulse_b);
        glm_vec3_sub(ret.torque_b, torque_b, ret.torque_b);

        //#if (CALCULATE_ROTATION)
        //        if (!rigidbody_a->disable_rotation) {}
        //        // TESTING
        //        // glm_vec3_sub(body_b_lin_vel, impulse, body_b_lin_vel);
        //        // glm_vec3_sub(body_b_ang_vel, torque, body_b_ang_vel);
        //#endif // (CALCULATE_ROTATION)
    }
#endif

// TODO: IMPORTANT (SKIPPING FRICTION)
#if 1
    return ret;
#endif

    // -----------------------------
    // Friction Step (re-calculate new impulse based on friction tangent)
    // -----------------------------

    // predicted
    vec3 pred_A_vel, pred_A_ang;
    vec3 pred_B_vel, pred_B_ang;
    glm_vec3_add(body_a.linear_velocity, impulse, pred_A_vel);
    glm_vec3_add(body_a.angular_velocity, torque, pred_A_ang);
    glm_vec3_cross(pred_A_ang, ra, ra_perp);

    // glm_vec3_sub(body_b.linear_velocity, impulse, pred_B_vel);
    // glm_vec3_sub(body_b.angular_velocity, torque, pred_B_ang);
    glm_vec3_copy(body_b.linear_velocity, pred_B_vel);
    glm_vec3_copy(body_b.angular_velocity, pred_B_ang);

    // calculate relative velocity
    {
        __calc_relative_velocity(solver_data,
                                 ra_perp,
                                 rb_perp,
                                 pred_A_vel,
                                 pred_A_ang,
                                 pred_B_vel,
                                 pred_B_ang,
                                 relative_velocity);
    }

    // create tangent
    vec3 tangent = GLM_VEC3_ZERO_INIT;
    {
// check tangent for near-zero
#if 0
        float zerodist = glm_vec3_distance(tangent, GLM_VEC3_ZERO);
        if (zerodist <= 0.00005f) { return; }
#else
        vec3 rvn, rvt;
        glm_vec3_scale(collision_normal,
                       glm_vec3_dot(relative_velocity, collision_normal),
                       rvn);
        glm_vec3_sub(relative_velocity, rvn, rvt);

        float rvt_len2 = glm_vec3_norm2(rvt);
        if (rvt_len2 < 1e-10f)
        {
            // Rolling resistance
            if (!rigidbody_a->disable_rotation)
            {
                float rolling = ROLLING_FRICTION;
                vec3 w        = GLM_VEC3_ZERO_INIT;
                glm_vec3_copy(rigidbody_a->angular_velocity, w);

                float wlen2 = glm_vec3_norm2(w);
                if (wlen2 > 1e-12f)
                {
                    glm_vec3_scale(w, 1.0f / sqrtf(wlen2), w); // unit spin axis
                    glm_vec3_scale(w, -rolling, w);            // resist spin
                    glm_vec3_add(rigidbody_a->torque, w, rigidbody_a->torque);
                }
            }

            // stick / no reliable tangent direction
            return ret;
        }

        glm_vec3_scale(
          rvt, 1.0f / sqrtf(rvt_len2), tangent); // tangent = normalized rvt
#endif

        // proceed with calculation for tangent
        glm_vec3_normalize(tangent);
    }

    // calculate Ft
    {
        vec3 raxt, rbxt;
        glm_vec3_cross(ra, tangent, raxt);
        glm_vec3_cross(rb, tangent, rbxt);

        f32 vDotT = (glm_vec3_dot(relative_velocity, tangent));
        if (vDotT < 0.0f) { glm_vec3_negate(tangent); }

#if 0
        f32 ra_perpDotT = glm_dot(ra_perp, tangent);
        f32 rb_perpDotT = glm_dot(rb_perp, tangent);

        f32 denom = body_a.inverse_mass + body_b.inverse_mass +
                    (pow(ra_perpDotT, 2) * body_a.inverse_inertia) +
                    (pow(rb_perpDotT, 2) * body_b.inverse_inertia);
#else
        f32 denom = body_a.inverse_mass + body_b.inverse_mass +
                    glm_vec3_dot(raxt, raxt) * body_a.inverse_inertia +
                    glm_vec3_dot(rbxt, rbxt) * body_b.inverse_inertia;
#endif

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

#if 0
        f32 friction_val = (fabsf(Ft) <= F * sf) ? Ft : -F * df;
#else
        // float jn = F;
        float jn = (F > 0.0f) ? F : 0.0f;
        float jt = Ft;

        // Static friction threshold and dynamic clamp
        float max_static = sf * jn;

        float friction_val =
          (fabsf(jt) <= max_static)
            ? jt
            : copysignf(df * jn, jt); // oppose tangential motion
#endif

        // create friction_impulse and friction_torque
        glm_vec3_scale(tangent, friction_val, friction_impulse);
        glm_vec3_cross(ra, friction_impulse, friction_torque);

// scale friction_impulse and friction_torque
#if 1
        glm_vec3_scale(friction_impulse, body_a.inverse_mass, friction_impulse);
        glm_vec3_scale(
          friction_torque, body_a.inverse_inertia, friction_torque);
#endif
    }

#if 1
    // apply friction impulses
    {
        //        glm_vec3_add(rigidbody_a->force_velocity,
        //                     friction_impulse,
        //                     rigidbody_a->force_velocity);
        //
        //#if (CALCULATE_ROTATION)
        //        if (!rigidbody_a->disable_rotation)
        //        {
        //            glm_vec3_add(
        //              rigidbody_a->torque, friction_torque,
        //              rigidbody_a->torque);
        //        }
        //#endif // (CALCULATE_ROTATION)

        glm_vec3_add(ret.impulse_a, friction_impulse, ret.impulse_a);
        glm_vec3_add(ret.torque_a, friction_torque, ret.torque_a);

        glm_vec3_sub(ret.impulse_b, friction_impulse, ret.impulse_b);
        glm_vec3_sub(ret.torque_b, friction_torque, ret.torque_b);
    }
#endif

    // --------------
    // DEBUG SOME LINES
    {
        if (p_debug_context->physics_options.draw_friction &&
            solver_data.entity.id == gsk_runtime_get_debug_entity_id())
        {
            //_BRK();
            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 8,
                                   solver_data.p_transform->position,
                                   relative_velocity,
                                   1,
                                   VCOL_CYAN,
                                   FALSE);

            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 9,
                                   points.point_a,
                                   friction_torque,
                                   10,
                                   VCOL_ORANGE,
                                   FALSE);

            gsk_debug_markers_push(p_debug_context,
                                   MARKER_RAY,
                                   solver_data.entity.id + 10,
                                   points.point_a,
                                   friction_impulse,
                                   10,
                                   VCOL_PURPLE,
                                   FALSE);
        }
    }

    return ret;
}
