/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: joint ECS system prepares joint data in update, sends to the physics
// solver list

// physics solver clears all joint solvers at the end of the run

// C_Rigidbody has list of constraints? or separate C_Joint?

// gsk_rigidbody_add_constraint(CONSTRAINT_TYPE, constraint_data);

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

gsk_ConstraintSolverOutput
gsk_physics_joint_velocity_solver(gsk_ConstraintSolverData solver_data);

#endif // __GSK_PHYSICS_JOINT_SOLVER_H__