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
    LOG_INFO("Distance: %f\tRadius: %f", ret.depth, a->radius);

    if (ret.depth <= (a->radius + b->distance)) { ret.has_collision = 1; }

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