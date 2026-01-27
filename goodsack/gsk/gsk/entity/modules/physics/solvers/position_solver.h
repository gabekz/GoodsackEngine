/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_POSITION_SOLVER_H__
#define __GSK_POSITION_SOLVER_H__

#include "entity/modules/physics/solvers/solver_data.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void
gsk_physics_position_solver(gsk_PhysicsSolverData solver_data, vec3 pos_fix);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_POSITION_SOLVER_H__