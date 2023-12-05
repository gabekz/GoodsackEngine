/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_solver.h"

#include <assert.h>

#include "physics/physics_types.inl"
#include "util/logger.h"

gsk_PhysicsSolver
gsk_physics_solver_init()
{
    gsk_PhysicsSolver ret = {
      .solvers       = malloc(sizeof(gsk_CollisionResult) * 64),
      .solvers_count = 64,
      .solver_next   = 0,
      .solver_empty  = TRUE,
    };

    return ret;
}

void
gsk_physics_solver_push(gsk_PhysicsSolver *solver, gsk_CollisionResult collision_result)
{
    if (solver->solver_next >= 64) LOG_CRITICAL("Exceeding solver capacity!");

    solver->solvers[solver->solver_next] = collision_result;
    solver->solver_next++;
    solver->solver_empty = FALSE;
}

void
gsk_physics_solver_pop(gsk_PhysicsSolver *solver)
{
    if (solver->solver_empty) {
        LOG_WARN("Trying to pop empty PhysicsSolver list");
        return; // nothing to pop.
    }

    if (((int)solver->solver_next - 1) < 0) {
        solver->solver_empty = TRUE;
        solver->solver_next  = 0;
        return;
    }
    solver->solver_next--;
}

void
gsk_physics_solver_step(gsk_PhysicsSolver *solver)
{
    if (solver->solver_empty) {
        assert(solver->solver_next == 0);
        return;
    }

    // TODO: run all solvers

    gsk_physics_solver_pop(solver); // pop when resolved
}
