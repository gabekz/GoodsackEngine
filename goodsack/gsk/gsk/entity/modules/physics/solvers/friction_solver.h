/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __FRICTION_SOLVER_H__
#define __FRICTION_SOLVER_H__

#include "entity/ecs.h"
#include "physics/physics_solver.h"

typedef struct _SolverData
{
    struct ComponentRigidbody *const p_rigidbody;
    struct ComponentTransform *const p_transform;
    gsk_CollisionResult *p_collision_result;
    const gsk_Entity entity;
    const f64 delta;
} _SolverData;

void
impulse_solver_with_rotation_friction(_SolverData solver_data);

#endif // __FRICTION_SOLVER_H__
