/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_solver.h"

#include <assert.h>

#include "physics/physics_types.inl"

#include "util/array_list.h"
#include "util/logger.h"

gsk_PhysicsSolver
gsk_physics_solver_init()
{
    gsk_PhysicsSolver ret;
    ret.solvers_list = malloc(sizeof(ArrayList));
    *(ArrayList *)ret.solvers_list =
      array_list_init(sizeof(gsk_CollisionResult));

    ret.solvers = ret.solvers_list->data.buffer;
    return ret;
}

void
gsk_physics_solver_push(gsk_PhysicsSolver *solver,
                        gsk_CollisionResult collision_result)
{
    array_list_push(solver->solvers_list, &collision_result);
}

void
gsk_physics_solver_pop(gsk_PhysicsSolver *solver)
{
    array_list_pop(solver->solvers_list);
}

void
gsk_physics_solver_step(gsk_PhysicsSolver *solver)
{
    if (solver->solvers_list->is_list_empty) {
        assert(solver->solvers_list->list_next == 0);
        return;
    }

    // TODO: run all solvers

    gsk_physics_solver_pop(solver); // pop when resolved
}
