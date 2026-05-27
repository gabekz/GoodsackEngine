/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_PHYSICS_JOINT_SOLVER_H__
#define __GSK_PHYSICS_JOINT_SOLVER_H__

#include "entity/modules/physics/solvers/solver_data.h"

#include "util/sysdefs.h"

// static void
// gsk_physics_joint_velocity_solver(gsk_JointSolverData data);

void
gsk_physics_distance_joint_position_solver(vec3 world_a,
                                           vec3 world_b,
                                           f32 inv_a,
                                           f32 inv_b,
                                           f32 rest_length,
                                           vec3 out_impulse);

void
gsk_physics_joint_velocity_solver(gsk_ConstraintSolverData solver_data);

#endif // __GSK_PHYSICS_JOINT_SOLVER_H__