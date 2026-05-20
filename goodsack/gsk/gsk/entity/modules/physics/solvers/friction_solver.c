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
static f32
__calc_effective_mass_along_normal(gsk_PhysicsSolverData solver_data,
                                   vec3 normal,
                                   vec3 ra,
                                   vec3 rb)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker                = collision_result->physics_mark;

    gsk_DynamicBody body_a = marker.body_a;
    gsk_DynamicBody body_b = marker.body_b;

    vec3 raxn, rbxn;
    glm_vec3_cross(ra, normal, raxn);
    glm_vec3_cross(rb, normal, rbxn);

    return body_a.inverse_mass + body_b.inverse_mass +
           (glm_vec3_dot(raxn, raxn) * body_a.inverse_inertia) +
           (glm_vec3_dot(rbxn, rbxn) * body_b.inverse_inertia);
}

gsk_PhysicsSolverLambda
gsk_physics_impulse_solver(gsk_PhysicsSolverData solver_data, f32 lambda_n)
{
    gsk_PhysicsSolverLambda ret = {0};

    gsk_DebugContext *p_debug_context =
      solver_data.entity.ecs->renderer->debugContext;

    gsk_CollisionResult *collision_result = solver_data.p_collision_result;

    vec3 relative_velocity, ra, rb;

    // calculate relative velocity
    __calc_relative_velocity(solver_data, relative_velocity, ra, rb);

    f32 F = 0.0f;
    vec3 collision_normal;
    glm_vec3_copy(
      collision_result->manifold.contacts[solver_data.contact_point].normal,
      collision_normal);

    // calculate F
    {
        float vDotN = (glm_vec3_dot(relative_velocity, collision_normal));

        f32 denom = __calc_effective_mass_along_normal(
          solver_data, collision_normal, ra, rb);

        F = -(1.0f + DEFAULT_RESTITUION) * vDotN;
        F = (denom != 0) ? F / denom : F;
        // if (vDotN > 0.0f) { F = 0.0f; }

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
    vec3 impulse = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(collision_normal, F, impulse);

    glm_vec3_copy(impulse, ret.impulse_total);
    return ret;
}
gsk_PhysicsSolverLambda
gsk_physics_friction_solver(gsk_PhysicsSolverData solver_data,
                            f32 lambda_n,
                            f32 lambda_t)
{

    gsk_PhysicsSolverLambda ret = {0};

    gsk_PhysicsMark marker = solver_data.p_collision_result->physics_mark;
    gsk_DynamicBody body_a = marker.body_a;
    gsk_DynamicBody body_b = marker.body_b;

    gsk_DebugContext *p_debug_context =
      solver_data.entity.ecs->renderer->debugContext;

    gsk_CollisionResult *collision_result = solver_data.p_collision_result;

    vec3 ra, rb = GLM_VEC3_ZERO_INIT;

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(
      collision_result->manifold.contacts[solver_data.contact_point].normal,
      collision_normal);

    vec3 relative_velocity = GLM_VEC3_ZERO_INIT;

    float Ft = 0;

    float restitution = DEFAULT_RESTITUION; // Bounce factor

    // -----------------------------
    // Friction Step (re-calculate new impulse based on friction tangent)
    // -----------------------------

    __calc_relative_velocity(solver_data, relative_velocity, ra, rb);
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
#if 0 // ROLLING FRICTION
        if (rvt_len2 < 1e-10f)
        {
            // TODO: might want to check if we want this as an option?
            // might break on player controller
            float rolling = ROLLING_FRICTION;
            vec3 w        = GLM_VEC3_ZERO_INIT;
            glm_vec3_copy(body_a.angular_velocity, w);

            float wlen2 = glm_vec3_norm2(w);
            if (wlen2 > 1e-12f)
            {
                glm_vec3_scale(w, 1.0f / sqrtf(wlen2), w); // unit spin axis
                glm_vec3_scale(w, -rolling, w);            // resist spin
                glm_vec3_add(ret.torque_a, w, ret.torque_a);
            }

            // stick / no reliable tangent direction
            return ret;
        }
#endif

        f32 divs = sqrtf(rvt_len2);
        if (fabsf(divs) < 1e-6f) { return ret; }

        glm_vec3_scale(rvt, 1.0f / divs, tangent); // tangent = normalized rvt
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
        // if (vDotT < 0.0f) { return ret; }

        f32 denom =
          __calc_effective_mass_along_normal(solver_data, tangent, ra, rb);

        Ft = vDotT;
        if (denom != 0) { Ft /= denom; }

        f32 delta_lambda = Ft;
        f32 old_lambda   = lambda_t;
        ret.lambda_t     = fmaxf(old_lambda + delta_lambda, 0.0f);

        // TODO: this is broken for some reason
        // NOTE: Not sure if this should be negative or not
        Ft = -(ret.lambda_t - old_lambda);
        // Ft = -delta_lambda;
    }

    // create friction_impulse, friction_torque
    float friction_val    = 0.0f;
    vec3 friction_impulse = GLM_VEC3_ZERO_INIT;
    {
        f32 sf = (body_a.static_friction + body_b.static_friction) * 0.5f;
        f32 df = (body_a.dynamic_friction + body_b.dynamic_friction) * 0.5f;

#if 0
        f32 friction_val = (fabsf(Ft) <= F * sf) ? Ft : -F * df;
#else
        // float jn = F;
        float jn = lambda_n;
        float jt = Ft;

        // Static friction threshold and dynamic clamp
        float max_static = sf * jn;

#if 0
        friction_val = (jt <= max_static)
                         ? jt
                         : copysignf(df * jn, jt); // oppose tangential motion

#else
        friction_val = (fabsf(jt) <= max_static)
                         ? jt
                         : copysignf(df * jn, jt); // oppose tangential motion
#endif
#endif
    }

    vec3 friction_impulse_a, friction_impulse_b;
    vec3 friction_torque_a, friction_torque_b;

    // create friction_impulse and friction_torque
    glm_vec3_scale(tangent, friction_val, friction_impulse);
    glm_vec3_copy(friction_impulse, ret.impulse_total);
    return ret;
}