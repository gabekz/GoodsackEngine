#include "physics_collision.h"

#include <util/logger.h>

// Sphere v. Sphere
CollisionPoints
physics_collision_find_sphere_sphere(SphereCollider *a,
                                     SphereCollider *b,
                                     vec3 pos_a,
                                     vec3 pos_b)
{
    return (CollisionPoints) {.has_collision = 0};
}

// Sphere v. Plane
CollisionPoints
physics_collision_find_sphere_plane(SphereCollider *a,
                                    PlaneCollider *b,
                                    vec3 pos_a,
                                    vec3 pos_b)
{
    CollisionPoints ret = {.has_collision = 0};

    // LOG_INFO("Checking sphere v plane");

    // LOG_INFO("%f\t%f\t%f", pos_a[0], pos_a[1], pos_a[2]);

    // Get distance of sphere->center to plane
    // if distance is less than sphere->distance, we have collision

    // distance of positions
    ret.depth = glm_vec3_distance(pos_a, pos_b);
    // LOG_INFO("Distance: %f\tRadius: %f", ret.depth, a->radius);

    // if (ret.depth <= (a->radius + b->distance)) { ret.has_collision = 1; }

    // Nearest point (Plane)
    vec3 planeNormalized = GLM_VEC3_ZERO_INIT;
    glm_vec3_normalize_to(pos_b, planeNormalized);

    vec3 normalizedPosA = GLM_VEC3_ZERO_INIT;
    glm_vec3_normalize_to(pos_a, normalizedPosA);

    float ndot  = glm_vec3_dot(pos_b, pos_a);
    float ndist = ndot - b->distance;
    // LOG_INFO("ndot: %f\tndist: %f", ndot, ndist);
    // LOG_INFO("%f\t%f\t%f", b->normal[0], b->normal[1], b->normal[2]);
    vec3 nscaledDist  = GLM_VEC3_ZERO_INIT;
    vec3 nearestPoint = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(planeNormalized, ndist, nscaledDist);
    // glm_vec3_normalize_to(nscaledDist, nscaledDist);
    glm_vec3_sub(pos_a, nscaledDist, nearestPoint);
    // glm_vec3_normalize_to(nearestPoint, nearestPoint);

#if 0
    LOG_INFO("NEAREST POINT: %f\t%f\t%f",
             nearestPoint[0],
             nearestPoint[1],
             nearestPoint[2]);
#endif

    /*
    if (nscaledDist <= a->radius) {
        // LOG_INFO("We have a collision!");
        ret.has_collision = TRUE;
    }
    */

    float nearestDistance = glm_vec3_distance(pos_a, nearestPoint);
    // LOG_INFO("Nearest: %f", nearestDistance);
    if (nearestDistance <= a->radius) { ret.has_collision = TRUE; }

    return ret;
}

// Plane v. Sphere
CollisionPoints
physics_collision_find_plane_sphere(PlaneCollider *a,
                                    SphereCollider *b,
                                    vec3 pos_a,
                                    vec3 pos_b)
{
    return (CollisionPoints) {.has_collision = 0};
}

// Raycast v. Sphere
CollisionPoints
physics_collision_find_ray_sphere(Raycast *ray,
                                  SphereCollider *sphere,
                                  vec3 pos_sphere)
{
    /*
    vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc,oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    return (discriminant>0);
    */

    CollisionPoints ret = {.has_collision = 0};

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