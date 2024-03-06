/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_collision.h"

#include "util/logger.h"

/*************************************************************************
 * Helper functions
 *************************************************************************/

static void
_aabb_clamped_point(vec3 bounds[2], vec3 point_a, vec3 point_b, float *dest)
{
    vec3 clamped, center;
    glm_aabb_center(bounds, center);

    f32 halfwidth = (bounds[1][0] - bounds[0][0]) / 2;
    clamped[0]    = MAX(-halfwidth, MIN(halfwidth, point_b[0] - point_a[0]));
    clamped[1]    = MAX(-halfwidth, MIN(halfwidth, point_b[1] - point_a[1]));
    clamped[2]    = MAX(-halfwidth, MIN(halfwidth, point_b[2] - point_a[2]));

    glm_vec3_add(center, clamped, dest);
}

static void
_invert_points(float *point_a, float *point_b)
{
    vec3 hold = GLM_VEC3_ONE_INIT;
    hold[0]   = point_a[0];
    hold[1]   = point_a[1];
    hold[2]   = point_a[2];
    glm_vec3_copy(point_b, point_a);
    glm_vec3_copy(hold, point_b);
}

static void
_closest_point_line_segment(vec3 a, vec3 b, vec3 point, float *dest)
{
    vec3 ab, p_a;
    glm_vec3_sub(b, a, ab);      // ab = B - A
    glm_vec3_sub(point, a, p_a); // p_a = Point - A

    f32 t = glm_dot(p_a, ab) / glm_dot(ab, ab);

    f32 offset = MIN(MAX(t, 0), 1); // saturate

    // return A + saturate(t) * AB
    glm_vec3_scale(ab, offset, dest);
    glm_vec3_add(a, dest, dest);
}

/*************************************************************************
 * Static functions - collision-tests with inverse
 *************************************************************************/

static gsk_CollisionPoints
__find_box_sphere_inverse(
  gsk_BoxCollider *a, gsk_SphereCollider *b, vec3 pos_a, vec3 pos_b, u8 inverse)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

    vec3 bounds[2];
    glm_vec3_add(pos_a, a->bounds[0], bounds[0]);
    glm_vec3_add(pos_a, a->bounds[1], bounds[1]);

    // calculate nearest point
    vec3 dist_vec;
    glm_vec3_sub(pos_b, pos_a, ret.normal);
    glm_vec3_normalize(ret.normal);

    // scale normal to radius for collision-test
    glm_vec3_scale(ret.normal, b->radius, dist_vec);
    glm_vec3_sub(pos_b, dist_vec, dist_vec);

    if (glm_aabb_point(bounds, dist_vec)) {
        ret.has_collision = TRUE;

        // calculate point_a
        _aabb_clamped_point(bounds, pos_a, pos_b, ret.point_a);

        // calculate point_b
        glm_vec3_scale(ret.normal, b->radius, ret.point_b);
        glm_vec3_sub(pos_b, ret.point_b, ret.point_b);

// possibly invert points
#if 1
        if (inverse) {
            _invert_points(ret.point_a, ret.point_b);
            // glm_vec3_negate(ret.normal);
        }
#endif

        // get new normal
        glm_vec3_sub(ret.point_b, ret.point_a, ret.normal);
        glm_vec3_normalize(ret.normal);

        // set depth
        ret.depth = glm_vec3_distance(ret.point_b, ret.point_a);
    }

    return ret;
}

/*************************************************************************
 * implementation from physics_collision.h
 *************************************************************************/

/*------------------------------------------------------------------------
 * Sphere
 ------------------------------------------------------------------------*/

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
        glm_vec3_sub(pos_b, pos_a, normal);
        glm_normalize(normal);
        glm_vec3_copy(normal, ret.normal);

        // calculate point_a
        glm_vec3_scale(normal, a->radius, ret.point_a);
        glm_vec3_add(pos_a, ret.point_a, ret.point_a);

        // calculate point_b
        glm_vec3_scale(normal, b->radius, ret.point_b);
        glm_vec3_sub(pos_b, ret.point_b, ret.point_b);

        glm_vec3_sub(ret.point_b, ret.point_a, ret.normal);
        glm_vec3_normalize(ret.normal);

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
    glm_vec3_sub(pos_b, pos_a, A);

    vec3 plane_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(((gsk_PlaneCollider *)b->plane)->normal, plane_normal);
    float nearestDistance = -glm_vec3_dot(plane_normal, A);

    // furthest point_a = pos_a - plane_normal * distance

    // NOTE: may need to use an offset for the radius (i.e, 0.02f)
    if (nearestDistance <= (a->radius)) {
        ret.has_collision = TRUE;
        glm_vec3_copy(plane_normal, ret.normal);

        // calculate point_a
        glm_vec3_scale(plane_normal, a->radius, ret.point_a);
        glm_vec3_sub(pos_a, ret.point_a, ret.point_a);

        // calculate point_b
        // TODO: should be normal of the collision, not plane_normal
        glm_vec3_scale(plane_normal, nearestDistance, ret.point_b);
        glm_vec3_sub(pos_a, ret.point_b, ret.point_b);

        // calculate normal of points
        glm_vec3_sub(ret.point_b, ret.point_a, ret.normal);
        glm_vec3_normalize(ret.normal);

        ret.depth = glm_vec3_distance(ret.point_b, ret.point_a);
    }

    return ret;
}

/*------------------------------------------------------------------------
 * Plane
 ------------------------------------------------------------------------*/

// Plane v. Sphere
gsk_CollisionPoints
gsk_physics_collision_find_plane_sphere(gsk_PlaneCollider *a,
                                        gsk_SphereCollider *b,
                                        vec3 pos_a,
                                        vec3 pos_b)
{
    return (gsk_CollisionPoints) {.has_collision = 0};
}

/*------------------------------------------------------------------------
 * Box
 ------------------------------------------------------------------------*/

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
    glm_vec3_sub(pos_b, pos_a, A);

    vec3 plane_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(((gsk_PlaneCollider *)b->plane)->normal, plane_normal);

    float nearestDistance = -glm_vec3_dot(plane_normal, A);

    vec3 bounds[2];
    glm_vec3_add(pos_a, a->bounds[0], bounds[0]);
    glm_vec3_add(pos_a, a->bounds[1], bounds[1]);

    f32 box_width = (bounds[1][0] - bounds[0][0]);

    if (nearestDistance <= box_width / 2) {

        ret.has_collision = TRUE;

        // calculate point_a
        glm_vec3_scale(plane_normal, box_width / 2, ret.point_a);
        glm_vec3_sub(pos_a, ret.point_a, ret.point_a);
        _aabb_clamped_point(bounds, pos_a, ret.point_a, ret.point_a);

        // calculate point_b
        glm_vec3_scale(plane_normal, nearestDistance, ret.point_b);
        glm_vec3_sub(pos_a, ret.point_b, ret.point_b);

        // calculate normal of points
        glm_vec3_sub(ret.point_b, ret.point_a, ret.normal);
        glm_vec3_normalize(ret.normal);

        ret.depth = glm_vec3_distance(ret.point_b, ret.point_a);
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

    return __find_box_sphere_inverse(a, b, pos_a, pos_b, FALSE);
}

// Box v. Box
gsk_CollisionPoints
gsk_physics_collision_find_box_box(gsk_BoxCollider *a,
                                   gsk_BoxCollider *b,
                                   vec3 pos_a,
                                   vec3 pos_b)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

    vec3 bounds_a[2], bounds_b[2];
    glm_vec3_add(pos_a, a->bounds[0], bounds_a[0]);
    glm_vec3_add(pos_a, a->bounds[1], bounds_a[1]);

    glm_vec3_add(pos_b, b->bounds[0], bounds_b[0]);
    glm_vec3_add(pos_b, b->bounds[1], bounds_b[1]);

    if (glm_aabb_aabb(bounds_a, bounds_b)) {
        ret.has_collision = TRUE;

        // calculate normal
        vec3 normal = GLM_VEC3_ZERO_INIT;
        glm_vec3_sub(pos_b, pos_a, normal);
        glm_normalize(normal);
        glm_vec3_copy(normal, ret.normal);

#if 0
        // calculate nearest point
        vec3 dist_vec_a, dist_vec_b;
        // scale normal to radius for collision-test
        glm_vec3_scale(ret.normal, 1 / 2, dist_vec_a);
        glm_vec3_add(pos_a, dist_vec_a, dist_vec_a);

        glm_vec3_scale(ret.normal, 1 / 2, dist_vec_b);
        glm_vec3_add(pos_b, dist_vec_b, dist_vec_b);
        // glm_vec3_negate(dist_vec_b);
#endif

        // calculate point a
        _aabb_clamped_point(bounds_a, pos_a, pos_b, ret.point_a);

        // calculate point b
        glm_vec3_sub(pos_b, ret.point_a, normal);
        glm_vec3_normalize(normal);

        float dist = glm_vec3_distance(pos_a, ret.point_a);
        glm_vec3_scale(normal, dist, ret.point_b);
        glm_vec3_sub(pos_b, ret.point_b, ret.point_b);
        _aabb_clamped_point(bounds_b, pos_b, ret.point_b, ret.point_b);

        glm_vec3_sub(ret.point_b, ret.point_a, ret.normal);
        glm_vec3_normalize(ret.normal);

        ret.depth = glm_vec3_distance(ret.point_b, ret.point_a);
    }

    return ret;
}

// Sphere v. Box
gsk_CollisionPoints
gsk_physics_collision_find_sphere_box(gsk_SphereCollider *a,
                                      gsk_BoxCollider *b,
                                      vec3 pos_a,
                                      vec3 pos_b)
{

    return __find_box_sphere_inverse(b, a, pos_b, pos_a, TRUE);
}

/*------------------------------------------------------------------------
 * Capsule
 ------------------------------------------------------------------------*/

// Capsule v. Capsule
gsk_CollisionPoints
gsk_physics_collision_find_capsule_capsule(gsk_CapsuleCollider *a,
                                           gsk_CapsuleCollider *b,
                                           vec3 pos_a,
                                           vec3 pos_b)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

    vec3 a_norm, a_line_end_offset, a_A, a_B; // capsule A data
    vec3 b_norm, b_line_end_offset, b_A, b_B; // capsulbe B data

    // base and tip adjusted to position
    vec3 a_tip, a_base, b_base, b_tip;
    glm_vec3_add(a->base, pos_a, a_base);
    glm_vec3_add(a->tip, pos_a, a_tip);
    glm_vec3_add(b->base, pos_b, b_base);
    glm_vec3_add(b->tip, pos_b, b_tip);

    // calculate capsule A Data
    {
        // a_norm
        glm_vec3_sub(a_tip, a_base, a_norm);
        glm_vec3_normalize(a_norm);
        // a_line_end_offset
        glm_vec3_scale(a_norm, a->radius, a_line_end_offset);
        // a_A
        glm_vec3_add(a_base, a_line_end_offset, a_A);
        // a_B
        glm_vec3_sub(a_base, a_line_end_offset, a_B);
    }

    // calculate capsule B Data
    {
        // a_norm
        glm_vec3_sub(b_tip, b_base, b_norm);
        glm_vec3_normalize(b_norm);
        // a_line_end_offset
        glm_vec3_scale(b_norm, b->radius, b_line_end_offset);
        // a_A
        glm_vec3_add(b_base, b_line_end_offset, b_A);
        // a_B
        glm_vec3_sub(b_base, b_line_end_offset, b_B);
    }

    // vectors between end points
    vec3 v0, v1, v2, v3;
    glm_vec3_sub(b_A, a_A, v0);
    glm_vec3_sub(b_B, a_A, v1);
    glm_vec3_sub(b_A, b_A, v2);
    glm_vec3_sub(b_B, a_B, v3);

    // squared distances
    f32 d0 = glm_dot(v0, v0);
    f32 d1 = glm_dot(v1, v1);
    f32 d2 = glm_dot(v2, v2);
    f32 d3 = glm_dot(v3, v3);

    vec3 best_a, best_b;
    if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1) {
        glm_vec3_copy(a_B, best_a);
    } else {
        glm_vec3_copy(a_A, best_a);
    }

    _closest_point_line_segment(b_A, b_B, best_a, best_b);
    _closest_point_line_segment(a_A, a_B, best_b, best_a);

    vec3 pen_normal;
    glm_vec3_sub(best_a, best_b, pen_normal);
    f32 len = glm_vec3_norm(pen_normal); // get length
    glm_vec3_normalize(pen_normal);      // normalize
    f32 pen_depth = a->radius + b->radius - len;

    ret.has_collision = (pen_depth > 0);

    if (ret.has_collision) {
        glm_vec3_sub(best_a, pen_normal, ret.point_a);
        glm_vec3_add(best_b, pen_normal, ret.point_b);

        glm_vec3_sub(ret.point_b, ret.point_a, ret.normal);
        glm_vec3_normalize(ret.normal);
        ret.depth = glm_vec3_distance(ret.point_a, ret.point_b);
    }

    return ret;
}

// Capsule v. Plane
gsk_CollisionPoints
gsk_physics_collision_find_capsule_plane(gsk_CapsuleCollider *a,
                                         gsk_PlaneCollider *b,
                                         vec3 pos_a,
                                         vec3 pos_b)
{
    gsk_CollisionPoints ret = {.has_collision = 0};

    vec3 a_norm, a_line_end_offset, a_A, a_B; // capsule A data
    vec3 b_norm, b_line_end_offset, b_A, b_B; // capsulbe B data

    // base and tip adjusted to position
    vec3 a_tip, a_base;
    glm_vec3_add(a->base, pos_a, a_base);
    glm_vec3_add(a->tip, pos_a, a_tip);

    // calculate capsule A Data
    {
        // a_norm
        glm_vec3_sub(a_tip, a_base, a_norm);
        glm_vec3_normalize(a_norm);
        // a_line_end_offset
        glm_vec3_scale(a_norm, a->radius, a_line_end_offset);
        // a_A
        glm_vec3_add(a_base, a_line_end_offset, a_A);
        // a_B
        glm_vec3_sub(a_base, a_line_end_offset, a_B);
    }

    // A = q - plane.p[0]
    vec3 A = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(pos_b, pos_a, A);

    vec3 plane_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(((gsk_PlaneCollider *)b->plane)->normal, plane_normal);
    float nearestDistance = -glm_vec3_dot(plane_normal, A);

    glm_vec3_scale(plane_normal, nearestDistance, ret.point_b);
    glm_vec3_sub(pos_a, ret.point_b, ret.point_b);

    // vectors between end points
    vec3 v0, v1;
    glm_vec3_sub(ret.point_b, a_A, v0);
    glm_vec3_sub(ret.point_b, a_B, v1);
    f32 d0 = glm_dot(v0, v0);
    f32 d1 = glm_dot(v1, v1);

    vec3 best_a;

    // get exact point
    _closest_point_line_segment(a_A, a_B, A, best_a);

    vec3 pen_normal;
    glm_vec3_sub(best_a, ret.point_b, pen_normal);
    f32 len = glm_vec3_norm(pen_normal); // get length
    glm_vec3_normalize(pen_normal);      // normalize
    f32 pen_depth = a->radius - len;

    ret.has_collision = (pen_depth > 0);

    if (ret.has_collision) {
        glm_vec3_sub(best_a, pen_normal, ret.point_a);
        glm_vec3_sub(ret.point_b, ret.point_a, ret.normal);
        glm_vec3_normalize(ret.normal);
        ret.depth = glm_vec3_distance(ret.point_a, ret.point_b);
    }

    return ret;
}

/*------------------------------------------------------------------------
 * Raycast
 ------------------------------------------------------------------------*/

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

// gsk_Raycast v. Box
// TODO: change return type to gsk_RaycastResult
gsk_CollisionPoints
gsk_physics_collision_find_ray_box(gsk_Raycast *ray,
                                   gsk_BoxCollider *box,
                                   vec3 pos_box)
{
    // algorithm mostly taken from: https://gamedev.stackexchange.com/a/18459

    gsk_CollisionPoints ret = {.has_collision = 0};

    // r.dir is unit direction vector of ray
    vec3 dirfrac = GLM_VEC3_ZERO_INIT;
    dirfrac[0]   = 1.0f / ray->direction[0];
    dirfrac[1]   = 1.0f / ray->direction[1];
    dirfrac[2]   = 1.0f / ray->direction[2];

    vec3 bounds_world[2];
    glm_vec3_add(pos_box, box->bounds[0], bounds_world[0]);
    glm_vec3_add(pos_box, box->bounds[1], bounds_world[1]);
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is
    // maximal corner r.org is origin of ray
    float t1 = (bounds_world[0][0] - ray->origin[0]) * dirfrac[0];
    float t2 = (bounds_world[1][0] - ray->origin[0]) * dirfrac[0];
    float t3 = (bounds_world[0][1] - ray->origin[1]) * dirfrac[1];
    float t4 = (bounds_world[1][1] - ray->origin[1]) * dirfrac[1];
    float t5 = (bounds_world[0][2] - ray->origin[2]) * dirfrac[2];
    float t6 = (bounds_world[1][2] - ray->origin[2]) * dirfrac[2];

    float t    = 0; // ray length
    float tmin = MAX(MAX(MIN(t1, t2), MIN(t3, t4)), MIN(t5, t6));
    float tmax = MIN(MIN(MAX(t1, t2), MAX(t3, t4)), MAX(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is
    // behind us
    if (tmax < 0) {
        t = tmax;
        return ret;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax) {
        t = tmax;
        return ret;
    }

    t                 = tmin;
    ret.has_collision = TRUE;

    // get contact point
    glm_vec3_scale(ray->direction, t, ret.point_a);
    glm_vec3_add(ray->origin, ret.point_a, ret.point_a);

    return ret;

#if 0
    LOG_INFO("Hit Position:\t%lf\t%lf\t%lf",
             hitPosition[0],
             hitPosition[1],
             hitPosition[2]);
    LOG_INFO("discriminant %f", discriminant);
#endif

    return ret;
}