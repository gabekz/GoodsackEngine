#include "physics_solver.h"

#include <assert.h>

#include <physics/physics_types.inl>
#include <util/logger.h>

PhysicsSolver
physics_solver_init()
{
    PhysicsSolver ret = {
      .solvers       = malloc(sizeof(CollisionResult) * 64),
      .solvers_count = 64,
      .solver_next   = 0,
      .solver_empty  = TRUE,
    };

    return ret;
}

void
physics_solver_push(PhysicsSolver *solver, CollisionResult collisionResult)
{
    if (solver->solver_next >= 64) LOG_CRITICAL("Exceeding solver capacity!");

    solver->solvers[solver->solver_next] = collisionResult;
    solver->solver_next++;
    solver->solver_empty = FALSE;
}

void
physics_solver_pop(PhysicsSolver *solver)
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
physics_solver_step(PhysicsSolver *solver)
{
    if (solver->solver_empty) {
        assert(solver->solver_next == 0);
        return;
    }

    // TODO: run all solvers

    physics_solver_pop(solver); // pop when resolved
}
