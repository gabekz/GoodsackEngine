/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_collision.h"

#include "util/logger.h"

// TODO: Rework collision-points calculation
// [0] Distance should not be sent as the depth. This results in non-predictable
// physics calculation.

// Sphere v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_sphere_sphere(gsk_SphereCollider *a,
                                         gsk_SphereCollider *b,
                                         vec3 pos_a,
                                         vec3 pos_b)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

    float distance = sqrt((pos_a[0] - pos_b[0]) * (pos_a[0] - pos_b[0]) +
                          (pos_a[1] - pos_b[1]) * (pos_a[1] - pos_b[1]) +
                          (pos_a[2] - pos_b[2]) * (pos_a[2] - pos_b[2]));

    ret.has_collision = (distance < a->radius + b->radius);

    // TODO: [0] calculate closest points, NOT positions.
    if (ret.has_collision) {
        vec3 normal = GLM_VEC3_ZERO_INIT;
        glm_vec3_sub(pos_a, pos_b, normal);
        glm_normalize(normal);
        glm_vec3_copy(normal, ret.normal);

        // calculate point_a
        glm_vec3_scale(normal, a->radius, ret.point_a);
        glm_vec3_sub(pos_a, ret.point_a, ret.point_a);

        // calculate point_b
        glm_vec3_scale(normal, b->radius, ret.point_b);
        glm_vec3_add(pos_b, ret.point_b, ret.point_b);

        ret.depth = glm_vec3_distance(ret.point_b, ret.point_a);
    }

    return ret;
}

// Sphere v. Plane
gsk_CollisionPoints
gsk_physics_collision_find_sphere_plane(gsk_SphereCollider *a,
                                        gsk_PlaneCollider *b,
                                        vec3 pos_a,
                                        vec3 pos_b)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

    // A = q - plane.p[0]
    vec3 A = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(pos_a, pos_b, A);

    vec3 plane_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(((gsk_PlaneCollider *)b->plane)->normal, plane_normal);
    float nearestDistance = glm_vec3_dot(A, plane_normal);

#if 0
    if(nearestDistance <= 1.2f)
        LOG_INFO("Nearest: %f", nearestDistance);
        LOG_INFO("%f", glm_vec3_distance(pos_a, pos_b));
#endif

    // furthest point_a = pos_a - plane_normal * distance

    // NOTE: may need to use an offset for the radius (i.e, 0.02f)
    if (nearestDistance <= (a->radius)) {
        ret.has_collision = TRUE;
        glm_vec3_copy(plane_normal, ret.normal);

        // LOG_INFO("%f", nearestDistance);
        // LOG_INFO("%f\t%f\t%f", A[0], A[1], A[2]);

        // calculate point_a
        glm_vec3_scale(plane_normal, a->radius, ret.point_a);
        glm_vec3_sub(pos_a, ret.point_a, ret.point_a);

        // calculate point_b
        // glm_vec3_subs(pos_a, nearestDistance, ret.point_b);
        // glm_vec3_copy(A, ret.point_b);

        // calculate point_b
        // glm_vec3_scale(plane_normal, a->radius, ret.point_b);
        // glm_vec3_sub(pos_a, ret.point_b, ret.point_b);

        // ret.depth = nearestDistance;
        ret.depth = -(nearestDistance - 0.2f);

        // attempt to get closest point
#if 0
        vec3 furthestA = GLM_VEC3_ZERO_INIT;
        glm_vec3_scale(plane_normal, nearestDistance, furthestA);
        glm_vec3_add(pos_a, furthestA, ret.point_a);
        //glm_vec3_subs(ret.point_a, 0.05, ret.point_a);
        glm_vec3_mul(plane_normal, ret.point_a, ret.point_a);
        LOG_INFO("%f\t%f\t%f", ret.point_a[0], ret.point_a[1], ret.point_a[2]);
#endif
    }

    return ret;
}

// Plane v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_plane_sphere(gsk_PlaneCollider *a,
                                        gsk_SphereCollider *b,
                                        vec3 pos_a,
                                        vec3 pos_b)
{
    return (gsk_CollisionPoints) {.has_collision = 0};
}

// Box v. Plane
gsk_CollisionPoints
gsk_physics_collision_find_box_plane(gsk_BoxCollider *a,
                                     gsk_PlaneCollider *b,
                                     vec3 pos_a,
                                     vec3 pos_b)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

    // A = q - plane.p[0]
    vec3 A = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(pos_a, pos_b, A);

    vec3 plane_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(((gsk_PlaneCollider *)b->plane)->normal, plane_normal);

    float nearestDistance = glm_vec3_dot(A, plane_normal);

    vec3 bounds[2];
    mat4 matrix = GLM_MAT4_IDENTITY_INIT;
    glm_translate(matrix, pos_a);
    glm_aabb_transform(a->bounds, matrix, bounds);

    float size_old = glm_aabb_size(a->bounds);
    float size_new = glm_aabb_size(bounds);
#if 0
    if (size_old >= size_new + 0.01f || size_old <= size_new + 0.01f) {
        LOG_ERROR("incorrect calculation of bounding box. %f != %f",
                  size_old,
                  size_new);
    }
#endif

    if (nearestDistance <= glm_aabb_radius(bounds) / 2) {

        // LOG_INFO("Box colliding");

        ret.has_collision = TRUE;
        glm_vec3_copy(plane_normal, ret.normal);
        ret.depth = -(nearestDistance) + (glm_aabb_radius(bounds) / 2);
    }

    return ret;
}

// Box v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_box_sphere(gsk_BoxCollider *a,
                                      gsk_SphereCollider *b,
                                      vec3 pos_a,
                                      vec3 pos_b)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

#if 1
    vec3 bounds[2];
    mat4 matrix = GLM_MAT4_IDENTITY_INIT;
    glm_translate(matrix, pos_a);
    glm_aabb_transform(a->bounds, matrix, bounds);
#endif

    vec4 sphere = {pos_b[0], pos_b[1], pos_b[2], 0.4f};
    if (glm_aabb_sphere(bounds, sphere)) {
        ret.has_collision = TRUE;

        glm_vec3_sub(pos_a, pos_b, ret.normal);
        glm_normalize(ret.normal);

        glm_vec3_scale(ret.normal, glm_aabb_radius(bounds), ret.point_a);
        glm_vec3_sub(pos_a, ret.point_a, ret.point_a);

        glm_vec3_scale(ret.normal, b->radius, ret.point_b);
        glm_vec3_add(pos_b, ret.point_b, ret.point_b);

        ret.depth = glm_vec3_distance(ret.point_b, ret.point_a);
    }

    return ret;
}

// gsk_Raycast v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_ray_sphere(gsk_Raycast *ray,
                                      gsk_SphereCollider *sphere,
                                      vec3 pos_sphere)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

    vec3 oc = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(ray->origin, pos_sphere, oc);
    float a = glm_vec3_dot(ray->direction, ray->direction);
    float b = 2.0f * glm_vec3_dot(oc, ray->direction);

    float c            = glm_vec3_dot(oc, oc) - sphere->radius * sphere->radius;
    float discriminant = (b * b) - (4 * a * c);

    vec3 hitPosition = GLM_VEC3_ZERO_INIT;
    glm_vec3_adds(ray->origin, discriminant, hitPosition);
    glm_vec3_mul(hitPosition, ray->direction, hitPosition);

#if 0
    LOG_INFO("Hit Position:\t%lf\t%lf\t%lf",
             hitPosition[0],
             hitPosition[1],
             hitPosition[2]);
    LOG_INFO("discriminant %f", discriminant);
#endif

    ret.has_collision = (discriminant > 0);
    return ret;
}
