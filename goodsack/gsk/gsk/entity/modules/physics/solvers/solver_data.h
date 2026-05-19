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
    u32 contact_point;
    const gsk_Entity entity;
} gsk_PhysicsSolverData;

typedef struct gsk_PhysicsSolverLambda
{
    vec3 impulse_a;
    vec3 impulse_b;
    vec3 torque_a;
    vec3 torque_b;
    f32 lambda_n;
    f32 lambda_t;

} gsk_PhysicsSolverLambda;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_PHYSICS_SOLVER_DATA_H__