/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_PHYSICS_SOLVER_DATA_H__
#define __GSK_PHYSICS_SOLVER_DATA_H__

#include "entity/ecs.h"
#include "physics/physics_types.h"

#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_PhysicsSolverData
{
    gsk_CollisionResult *p_collision_result;
    const f64 delta;
    const gsk_Entity entity;
    u32 contact_point;
} gsk_PhysicsSolverData;

typedef struct gsk_PhysicsSolverLambda
{
    f32 lambda_n;
    f32 lambda_t;

    vec3 impulse_total;

} gsk_PhysicsSolverLambda;

typedef struct gsk_ConstraintSolverData
{
    const f64 delta;

    f64 beta;
    f64 softness;

    gsk_PhysicsMark physics_mark;

    vec3 local_anchor_a;
    vec3 local_anchor_b;

    vec3 world_a;
    vec3 world_b;

} gsk_ConstraintSolverData;

typedef struct gsk_ConstraintSolverOutput
{
    vec3 impulse_axes[3];
    vec3 ra;
    vec3 rb;
} gsk_ConstraintSolverOutput;

#if 0
typedef struct gsk_PhysicsConstraint
{
    // gsk_PhysicsConstraintType type;

    gsk_Entity entity_a;
    gsk_Entity entity_b;

    /*
     * Local-space anchor positions relative to each body's transform.
     * Example:
     *   local_anchor_a = elbow position in upper-arm local space
     *   local_anchor_b = elbow position in forearm local space
     */
    vec3 local_anchor_a;
    vec3 local_anchor_b;

    /*
     * Solver tuning.
     */
    float beta;     // Baumgarte bias factor, e.g. 0.1 - 0.3
    float softness; // optional CFM/softness, start at 0
    float max_impulse;

    /*
     * Warm starting / accumulated impulse.
     * Ball socket has 3 linear constraint axes.
     */
    vec3 accumulated_impulse;
} gsk_PhysicsConstraint;

typedef struct gsk_JointSolverData
{
    gsk_JointConstraint *joint;

    f32 inverse_mass_a;
    f2 inverse_mass_b;

    struct ComponentTransform *tr_a;
    struct ComponentTransform *tr_b;

    float delta;
} gsk_JointSolverData;
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_PHYSICS_SOLVER_DATA_H__