/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "position_solver.h"

void
gsk_physics_position_solver(gsk_PhysicsSolverData solver_data, vec3 pos_fix)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    struct ComponentTransform *transform  = solver_data.p_transform;

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(
      collision_result->manifold.contacts[solver_data.contact_point].normal,
      collision_normal);

#if 1
    // I think these are better settings right now..
    const float percent = 0.5f;
    const float slop    = 0.005f;
#else
    const float percent = 0.8f;
    const float slop    = 0.1f;
#endif

    vec3 correction;
    f32 c_weight = fmax(
      (collision_result->manifold.contacts[solver_data.contact_point].depth -
       slop),
      0);
    glm_vec3_scale(collision_normal, percent, correction);
    glm_vec3_scale(correction, c_weight, correction);

    // integrate new position
    glm_vec3_add(pos_fix, correction, pos_fix);
}