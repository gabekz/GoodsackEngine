/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PHYSICS_COLLISION_H__
#define __PHYSICS_COLLISION_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#include "physics/physics_types.inl"

gsk_Collider
gsk_physics_collider_new(u16 type);

// general?
gsk_CollisionPoints
gsk_physics_test_collision(gsk_Collider *a,
                           gsk_Collider *b,
                           vec3 pos_a,
                           vec3 pos_b);

// Sphere v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_sphere_sphere(gsk_SphereCollider *a,
                                         gsk_SphereCollider *b,
                                         vec3 pos_a,
                                         vec3 pos_b);

// Sphere v. Plane
gsk_CollisionPoints
gsk_physics_collision_find_sphere_plane(gsk_SphereCollider *a,
                                        gsk_PlaneCollider *b,
                                        vec3 pos_a,
                                        vec3 pos_b);

// Plane v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_plane_sphere(gsk_PlaneCollider *a,
                                        gsk_SphereCollider *b,
                                        vec3 pos_a,
                                        vec3 pos_b);

// Box v. Plane
gsk_CollisionPoints
gsk_physics_collision_find_box_plane(gsk_BoxCollider *a,
                                     gsk_PlaneCollider *b,
                                     vec3 pos_a,
                                     vec3 pos_b);

// gsk_Raycast v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_ray_sphere(gsk_Raycast *ray,
                                      gsk_SphereCollider *sphere,
                                      vec3 pos_sphere);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PHYSICS_COLLISION_H__