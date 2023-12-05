/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PHYSICS_SOLVER_H__
#define __PHYSICS_SOLVER_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#include "physics/physics_types.inl"

typedef struct gsk_PhysicsSolver
{
    gsk_CollisionResult *solvers;
    u64 solvers_count, solver_next;
    u16 solver_empty;
} gsk_PhysicsSolver;

gsk_PhysicsSolver
gsk_physics_solver_init();

void
gsk_physics_solver_push(gsk_PhysicsSolver *solver,
                        gsk_CollisionResult collision_result);

void
gsk_physics_solver_pop(gsk_PhysicsSolver *solver);

void
gsk_physics_solver_step(gsk_PhysicsSolver *solver);

#endif // __PHYSICS_SOLVER_H__
