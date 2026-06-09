/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_solver.h"

#include <assert.h>

#include "entity/ecs.h"
#include "physics/physics_types.h"

#include "util/array_list.h"
#include "util/logger.h"

#define LIST_INCREMENT_SIZE 8

static gsk_PhysicsSolver *s_solver = NULL;

void
gsk_physics_solver_init()
{
    gsk_PhysicsSolver *solver = malloc(sizeof(gsk_PhysicsSolver));
    s_solver                  = solver;

    s_solver->solvers_list = malloc(sizeof(ArrayList));
    *(ArrayList *)s_solver->solvers_list =
      array_list_init(sizeof(gsk_CollisionResult), LIST_INCREMENT_SIZE);

    s_solver->constraints_list = malloc(sizeof(ArrayList));
    *(ArrayList *)s_solver->constraints_list =
      array_list_init(sizeof(gsk_ConstraintResult), LIST_INCREMENT_SIZE);
}

gsk_PhysicsSolver *
gsk_physics_solver_get()
{
    return s_solver;
}

void
gsk_physics_solver_push(gsk_CollisionResult collision_result)
{
    array_list_push(s_solver->solvers_list, &collision_result);
}

void
gsk_physics_solver_push_constraint(gsk_ConstraintResult constraint_result)
{
    array_list_push(s_solver->constraints_list, &constraint_result);
}

void
gsk_physics_solver_clear()
{
    if (s_solver == NULL) { return; }

    while (s_solver->solvers_list->is_list_empty == FALSE)
    {
        array_list_pop(s_solver->solvers_list);
    }
}

void
gsk_physics_solver_clear_constraints()
{
    if (s_solver == NULL) { return; }

    while (s_solver->constraints_list->is_list_empty == FALSE)
    {
        array_list_pop(s_solver->constraints_list);
    }
}

u8
gsk_physics_solver_exists(gsk_EntityId entity_a, gsk_EntityId entity_b)
{
    if (s_solver->solvers_list->is_list_empty) { return FALSE; }

    for (int i = 0; i < s_solver->solvers_list->list_next; i++)
    {
        gsk_CollisionResult *p_result = LIST_GET(s_solver->solvers_list, i);

        if ((p_result->ent_a_id == entity_a &&
             p_result->ent_b_id == entity_b) ||
            (p_result->ent_a_id == entity_b && p_result->ent_b_id == entity_a))
        {
            return TRUE;
        }
    }

    return FALSE;
}
