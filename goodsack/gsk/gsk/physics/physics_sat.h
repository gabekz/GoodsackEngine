/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_PHYSICS_SAT_H__
#define __GSK_PHYSICS_SAT_H__

#include "physics/physics_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// 1. physics_sat.h [ X ]
// 2. obb-ray check
// 3. obb-sphere?
// 4. obb-obb multiple points

gsk_OBB
gsk_physics_sat_obb_make(const gsk_BoxCollider *col, vec3 pos, mat3 rot);

gsk_OBBSatResult
gsk_physics_sat_obb_test(gsk_OBB *a, gsk_OBB *b);

void
gsk_physics_sat_contact_single(gsk_OBB *a,
                               gsk_OBB *b,
                               const gsk_OBBSatResult *sat,
                               gsk_CollisionPoints *out);

gsk_CollisionManifold
gsk_physics_sat_find_obb_obb_manifold(gsk_OBB *a, gsk_OBB *b);

gsk_CollisionPoints
gsk_pyhysics_sat_find_obb_sphere_points(const gsk_OBB *box,
                                        vec3 sphere_center,
                                        float sphere_radius);

gsk_CollisionPoints
gsk_physics_sat_find_obb_capsule(const gsk_OBB *box,
                                 vec3 cap_a,
                                 vec3 cap_b,
                                 float radius);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_PHYSICS_SAT_H__