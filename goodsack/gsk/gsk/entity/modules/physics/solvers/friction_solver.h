/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_FRICTION_SOLVER_H__
#define __GSK_FRICTION_SOLVER_H__

#include "entity/modules/physics/solvers/solver_data.h"

#include "util/sysdefs.h"

gsk_PhysicsSolverLambda
gsk_physics_impulse_solver(gsk_PhysicsSolverData solver_data, f32 lambda_n);

gsk_PhysicsSolverLambda
gsk_physics_friction_solver(gsk_PhysicsSolverData solver_data,
                            f32 lambda_n,
                            f32 lambda_t);

#endif // __GSK_FRICTION_SOLVER_H__
