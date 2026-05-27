/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "joint_solver.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#include "entity/modules/physics/solvers/solver_data.h"

#include "entity/modules/transform/transform.h"

void
gsk_physics_distance_joint_position_solver(vec3 world_a,
                                           vec3 world_b,
                                           f32 inv_mass_a,
                                           f32 inv_mass_b,
                                           f32 rest_length,
                                           vec3 out_impulse)
{
    // TODO: make C_CONSTRAINT or add a list to rigidbody to make constraints
    // types: Point, Joint

    vec3 delta = GLM_VEC3_ZERO_INIT;

    // NOTE: must go from local-world passing into this fn
    glm_vec3_sub(world_a, world_b, delta);

    float len = glm_vec3_norm(delta);
    if (len < 1e-6f) return;

    vec3 n;
    glm_vec3_scale(delta, 1.0f / len, n);

    float error = len - rest_length;

    float inv_mass_sum = inv_mass_a + inv_mass_b;
    if (inv_mass_sum <= 0.0f) return;

    float percent = 0.8f;

    glm_vec3_scale(n, percent * error / inv_mass_sum, out_impulse);
    return;

#if 0
    if (!data.rb_a->is_kinematic)
    {
        vec3 move_a;
        glm_vec3_scale(correction, inv_mass_a, move_a);
        glm_vec3_add(data.tr_a->position, move_a, data.tr_a->position);
    }

    if (!data.rb_b->is_kinematic)
    {
        vec3 move_b;
        glm_vec3_scale(correction, inv_mass_b, move_b);
        glm_vec3_sub(data.tr_b->position, move_b, data.tr_b->position);
    }
#endif
}

void
gsk_physics_joint_velocity_solver(gsk_ConstraintSolverData solver_data)
{
    gsk_DynamicBody body_a = solver_data.physics_mark.body_a;
    gsk_DynamicBody body_b = solver_data.physics_mark.body_b;

    vec3 pa, pb;
    //_transform_local_point(data.tr_a, joint->local_anchor_a, pa);
    //_transform_local_point(data.tr_b, joint->local_anchor_b, pb);
    glm_vec3_copy(solver_data.world_a, pa);
    glm_vec3_copy(solver_data.world_b, pb);

    vec3 ra, rb;
    glm_vec3_sub(pa, body_a.position, ra);
    glm_vec3_sub(pb, body_b.position, rb);

    vec3 va_anchor;
    vec3 vb_anchor;

    vec3 wa_cross_ra;
    vec3 wb_cross_rb;

    glm_vec3_cross(body_a.angular_velocity, ra, wa_cross_ra);
    glm_vec3_cross(body_b.angular_velocity, rb, wb_cross_rb);

    glm_vec3_add(body_a.linear_velocity, wa_cross_ra, va_anchor);
    glm_vec3_add(body_b.linear_velocity, wb_cross_rb, vb_anchor);

    vec3 rel_v;
    glm_vec3_sub(vb_anchor, va_anchor, rel_v);

    /*
     * Baumgarte bias: correct positional error through velocity solve.
     */
    vec3 error;
    glm_vec3_sub(pb, pa, error);

    vec3 bias;
    glm_vec3_scale(error, solver_data.beta / solver_data.delta, bias);

    /*
     * Solve each world axis independently.
     * This is not perfect, but it is a good first implementation.
     */
    vec3 axes[3] = {
      {1.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 1.0f},
    };

    for (int i = 0; i < 3; ++i)
    {
        vec3 axis;
        glm_vec3_copy(axes[i], axis);

        float jv = glm_vec3_dot(rel_v, axis);
        float b  = glm_vec3_dot(bias, axis);

        /*
         * Approximate effective mass.
         * This ignores the full rotational Jacobian matrix.
         * Good enough to get a prototype moving.
         */
        float effective_mass = body_a.inverse_mass + body_b.inverse_mass;

        if (effective_mass <= 1e-6f) continue;

        float lambda = -(jv + b) / (effective_mass + solver_data.softness);

        vec3 impulse;
        glm_vec3_scale(axis, lambda, impulse);

        /*
         * A receives -impulse, B receives +impulse.
         */
        vec3 neg_impulse;
        glm_vec3_copy(impulse, neg_impulse);
        glm_vec3_negate(neg_impulse);

        //_apply_impulse_at_point(data.rb_a, neg_impulse, ra);
        //_apply_impulse_at_point(data.rb_b, impulse, rb);
    }
}