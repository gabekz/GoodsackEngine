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

    // A = q - plane.p[0]
    vec3 A = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(pos_a, pos_b, A);

    glm_vec3_normalize_to(A, A); // A is now normalized

    float angle = glm_vec3_dot(pos_a, pos_b);

    vec3 nearestPoint = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(A, angle, nearestPoint);

    float nearestDistance = glm_vec3_distance(A, nearestPoint);

#if 0
    if(nearestDistance <= 1.2f)
        LOG_INFO("Nearest: %f", nearestDistance);
#endif

    // NOTE: may need to use an offset for the radius (i.e, 0.02f)
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
