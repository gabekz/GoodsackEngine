#ifndef H_PHYSICS_SOLVER_H
#define H_PHYSICS_SOLVER_H

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
physics_resolver_init();

void
physics_solver_push(PhysicsSolver *solver, CollisionResult *collisionResult);

void
physics_solver_pop(PhysicsSolver *solver);

void
physics_solver_step(PhysicsSolver *solver);

#endif // H_PHYSICS_SOLVER_H