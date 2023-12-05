/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PHYSICS_SOLVER_H__
#define __PHYSICS_SOLVER_H__

#include <util/maths.h>
#include <util/sysdefs.h>

#include <physics/physics_types.inl>

typedef struct PhysicsSolver_t
{
    CollisionResult *solvers;
    ui64 solvers_count, solver_next;
    ui16 solver_empty;
} PhysicsSolver;

PhysicsSolver
physics_solver_init();

void
physics_solver_push(PhysicsSolver *solver, CollisionResult collisionResult);

void
physics_solver_pop(PhysicsSolver *solver);

void
physics_solver_step(PhysicsSolver *solver);

#endif // __PHYSICS_SOLVER_H__
