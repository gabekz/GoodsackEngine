/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PHYSICS_TYPES_INL__
#define __PHYSICS_TYPES_INL__

#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct CollisionPoints_t
{
    vec3 point_a;       // furthest point of A into B
    vec3 point_b;       // furthest point of B into A
    vec3 normal;        // point_b - point_a normalized
    float depth;        // Length of point_b - point_a
    ui16 has_collision; // bool
} CollisionPoints;

typedef struct CollisionResult_t
{
    void *object_a;
    void *object_b;
    CollisionPoints points;
} CollisionResult;

typedef struct Collider_t
{
    void *collider_data;
    ui16 collider_data_type;

    vec3 position;

    ui16 is_dynamic, is_trigger;
} Collider;

typedef struct SphereCollider_t
{
    vec3 center;
    float radius;
} SphereCollider;

typedef struct PlaneCollider_t
{
    vec3 plane, normal;
    float distance;
} PlaneCollider;

typedef struct BoxCollider_t
{
    vec3 bounds[2];
} BoxCollider;

typedef struct Raycast_t
{
    vec3 origin, direction;
} Raycast;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PHYSICS_TYPES_INL__
