/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PHYSICS_TYPES_H__
#define __PHYSICS_TYPES_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*************************************************************************
 * Physics types
 *************************************************************************/

typedef enum GskColliderType {
    COLLIDER_NONE = 0,
    COLLIDER_SPHERE,
    COLLIDER_PLANE,
    COLLIDER_BOX,
    COLLIDER_RAYCAST
} GskColliderType;

typedef struct gsk_CollisionPoints
{
    vec3 point_a;      // furthest point of A into B
    vec3 point_b;      // furthest point of B into A
    vec3 normal;       // point_b - point_a normalized
    float depth;       // Length of point_b - point_a
    u16 has_collision; // bool
} gsk_CollisionPoints;

typedef struct gsk_CollisionResult
{
    void *object_a;
    void *object_b;
    gsk_CollisionPoints points;
} gsk_CollisionResult;

typedef struct gsk_Collider
{
    void *collider_data;
    u16 collider_data_type;

    vec3 position;

    u16 is_dynamic, is_trigger;
} gsk_Collider;

typedef struct gsk_SphereCollider
{
    vec3 center;
    float radius;
} gsk_SphereCollider;

typedef struct gsk_PlaneCollider
{
    vec3 plane, normal;
    float distance;
} gsk_PlaneCollider;

typedef struct gsk_BoxCollider
{
    vec3 bounds[2];
} gsk_BoxCollider;

typedef struct gsk_Raycast
{
    vec3 origin, direction;
} gsk_Raycast;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PHYSICS_TYPES_H__