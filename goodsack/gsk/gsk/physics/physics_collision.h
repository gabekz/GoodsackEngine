/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PHYSICS_COLLISION_H__
#define __PHYSICS_COLLISION_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#include "physics/physics_types.h"

gsk_Collider
gsk_physics_collider_new(u16 type);

// general?
gsk_CollisionPoints
gsk_physics_test_collision(gsk_Collider *a,
                           gsk_Collider *b,
                           vec3 pos_a,
                           vec3 pos_b);

/*************************************************************************
 * Sphere definitions
 *************************************************************************/

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

// Sphere v. Box
gsk_CollisionPoints
gsk_physics_collision_find_sphere_box(gsk_SphereCollider *a,
                                      gsk_BoxCollider *b,
                                      vec3 pos_a,
                                      vec3 pos_b);

// Sphere v. Capsule
gsk_CollisionPoints
gsk_physics_collision_find_sphere_capsule(gsk_SphereCollider *a,
                                          gsk_CapsuleCollider *b,
                                          vec3 pos_a,
                                          vec3 pos_b);

/*************************************************************************
 * Plane definitions
 *************************************************************************/

// Plane v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_plane_sphere(gsk_PlaneCollider *a,
                                        gsk_SphereCollider *b,
                                        vec3 pos_a,
                                        vec3 pos_b);

/*************************************************************************
 * Box definitions
 *************************************************************************/

// Box v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_box_sphere(gsk_BoxCollider *a,
                                      gsk_SphereCollider *b,
                                      vec3 pos_a,
                                      vec3 pos_b);

// Box v. Box
gsk_CollisionPoints
gsk_physics_collision_find_box_box(gsk_BoxCollider *a,
                                   gsk_BoxCollider *b,
                                   vec3 pos_a,
                                   vec3 pos_b);

// Box v. Plane
gsk_CollisionPoints
gsk_physics_collision_find_box_plane(gsk_BoxCollider *a,
                                     gsk_PlaneCollider *b,
                                     vec3 pos_a,
                                     vec3 pos_b);

#if 0
// Box v. Capsule
gsk_CollisionPoints
gsk_physics_collision_find_box_capsule(gsk_BoxCollider *a,
                                       gsk_CapsuleCollider *b,
                                       vec3 pos_a,
                                       vec3 pos_b);
#endif

/*************************************************************************
 * Capsule definitions
 *************************************************************************/

// Capsule v. Capsule
gsk_CollisionPoints
gsk_physics_collision_find_capsule_capsule(gsk_CapsuleCollider *a,
                                           gsk_CapsuleCollider *b,
                                           vec3 pos_a,
                                           vec3 pos_b);

// Capsule v. Plane
gsk_CollisionPoints
gsk_physics_collision_find_capsule_plane(gsk_CapsuleCollider *a,
                                         gsk_PlaneCollider *b,
                                         vec3 pos_a,
                                         vec3 pos_b);

// Capsule v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_capsule_sphere(gsk_CapsuleCollider *a,
                                          gsk_SphereCollider *b,
                                          vec3 pos_a,
                                          vec3 pos_b);

#if 0
// Capsule v. Box
gsk_CollisionPoints
gsk_physics_collision_find_capsule_box(gsk_CapsuleCollider *a,
                                       gsk_BoxCollider *b,
                                       vec3 pos_a,
                                       vec3 pos_b);
#endif

/*************************************************************************
 * Raycast definitions
 *************************************************************************/

// gsk_Raycast v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_ray_sphere(gsk_Raycast *ray,
                                      gsk_SphereCollider *sphere,
                                      vec3 pos_sphere);

// gsk_Raycast v. Box
gsk_CollisionPoints
gsk_physics_collision_find_ray_box(gsk_Raycast *ray,
                                   gsk_BoxCollider *box,
                                   vec3 pos_box);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PHYSICS_COLLISION_H__
