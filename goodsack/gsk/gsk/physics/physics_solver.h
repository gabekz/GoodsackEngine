/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PHYSICS_SOLVER_H__
#define __PHYSICS_SOLVER_H__

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "entity/ecs.h"

#include "physics/physics_types.h"

typedef struct gsk_PhysicsSolver
{
    gsk_CollisionResult *solvers;
    ArrayList *solvers_list;
} gsk_PhysicsSolver;

void
gsk_physics_solver_init();

gsk_PhysicsSolver *
gsk_physics_solver_get();

void
gsk_physics_solver_push(gsk_CollisionResult collision_result);

void
gsk_physics_solver_pop();

void
gsk_physics_solver_step();

u8
gsk_physics_solver_exists(gsk_EntityId entity_a, gsk_EntityId entity_b);

#endif // __PHYSICS_SOLVER_H__
