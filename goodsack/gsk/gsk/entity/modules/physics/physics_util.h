/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_PHYSICS_UTIL_H__
#define __GSK_PHYSICS_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "entity/ecs.h"
#include "physics/physics_types.h"

gsk_PhysicsMark
gsk_physics_util_create_physics_mark(gsk_Entity entity_a, gsk_Entity entity_b);

void
gsk_physics_util_inverse_inertia_world(mat3 rot,
                                       mat3 inv_inertia_local,
                                       mat3 out_inv_inertia_world);

void
gsk_physics_util_relative_velocity(gsk_DynamicBody body_a,
                                   gsk_DynamicBody body_b,
                                   vec3 point_a,
                                   vec3 point_b,
                                   vec3 out_relative_velocity,
                                   vec3 out_ra,
                                   vec3 out_rb);

f32
gsk_physics_util_effective_mass_axis(gsk_DynamicBody *body_a,
                                     gsk_DynamicBody *body_b,
                                     vec3 ra,
                                     vec3 rb,
                                     vec3 axis);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__GSK_PHYSICS_UTIL_H__