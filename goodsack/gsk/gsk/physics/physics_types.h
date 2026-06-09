/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PHYSICS_TYPES_H__
#define __PHYSICS_TYPES_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#define GSK_MAX_COLLISION_CONTACTS 8

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
    COLLIDER_CAPSULE,
    COLLIDER_RAYCAST
} GskColliderType;

typedef struct gsk_CollisionPoints
{
    vec3 point_a; // furthest point of A into B
    vec3 point_b; // furthest point of B into A
    vec3 normal;  // point_b - point_a normalized
    f32 depth;    // Length of point_b - point_a
    f32 penetration;
    u16 has_collision; // bool
    f32 lambda_n;
    f32 lambda_t;

    vec3 local_point_a;
    vec3 local_point_b;

} gsk_CollisionPoints;

typedef struct gsk_CollisionManifold
{
    vec3 normal;
    f32 depth;
    u8 has_collision;

    u32 contacts_count;
    gsk_CollisionPoints contacts[GSK_MAX_COLLISION_CONTACTS];

} gsk_CollisionManifold;

typedef struct gsk_DynamicBody
{
    vec3 position;
    vec3 center_of_mass;
    vec3 linear_velocity;
    vec3 angular_velocity;
    f32 mass, inverse_mass;
    f32 inertia, inverse_inertia;
    f32 static_friction, dynamic_friction;

    mat3 inertia_tensor;

} gsk_DynamicBody;

typedef struct gsk_PhysicsMark
{
    gsk_DynamicBody body_a;
    gsk_DynamicBody body_b;
    vec3 relative_velocity; // velocity of b - a
} gsk_PhysicsMark;

typedef struct gsk_CollisionResult
{
    gsk_CollisionManifold manifold;
    gsk_PhysicsMark physics_mark;
    u64 ent_a_id, ent_b_id;
    u8 is_trigger_response;
} gsk_CollisionResult;

typedef struct gsk_ConstraintResult
{
    u64 ent_a_id;
    u64 ent_b_id;

    vec3 local_anchor_a;
    vec3 local_anchor_b;

    f64 softness;
    f64 rest_distance;

} gsk_ConstraintResult;

typedef struct gsk_Collider
{
    void *collider_data;
    u16 collider_data_type;

    vec3 center;

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
    vec3 center;
    mat3 rotation;
} gsk_BoxCollider;

typedef struct gsk_CapsuleCollider
{
    vec3 base, tip;
    f32 radius;
} gsk_CapsuleCollider;

typedef struct gsk_Raycast
{
    vec3 origin, direction;
} gsk_Raycast;

typedef struct gsk_OBB
{
    vec3 c;    // center
    vec3 e;    // half extents
    vec3 u[3]; // axes in world (unit)

    // testing
    vec3 com; // center-of-mass
    vec3 local_com;
} gsk_OBB;

typedef enum gsk_OBBAxisType {
    GSK_OBB_AXIS_NONE = 0,
    GSK_OBB_AXIS_FACE_A,
    GSK_OBB_AXIS_FACE_B,
    GSK_OBB_AXIS_EDGE
} gsk_OBBAxisType;

typedef struct gsk_OBBSatResult
{
    u8 has_collision;

    vec3 normal; // world-space normal from A -> B
    float depth;

    gsk_OBBAxisType type;
    int axis_a;
    int axis_b;
} gsk_OBBSatResult;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PHYSICS_TYPES_H__
