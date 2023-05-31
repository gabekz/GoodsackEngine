#ifndef H_PHYSICS_COLLISION
#define H_PHYSICS_COLLISION

#include <util/maths.h>
#include <util/sysdefs.h>

#include <physics/physics_types.inl>

Collider
physics_collider_new(ui16 type);

// general?
CollisionPoints
physics_test_collision(Collider *a, Collider *b, vec3 pos_a, vec3 pos_b);

// Sphere v. Sphere
CollisionPoints
physics_collision_find_sphere_sphere(SphereCollider *a,
                                     SphereCollider *b,
                                     vec3 pos_a,
                                     vec3 pos_b);

// Sphere v. Plane
CollisionPoints
physics_collision_find_sphere_plane(SphereCollider *a,
                                    PlaneCollider *b,
                                    vec3 pos_a,
                                    vec3 pos_b);

// Plane v. Sphere
CollisionPoints
physics_collision_find_plane_sphere(PlaneCollider *a,
                                    SphereCollider *b,
                                    vec3 pos_a,
                                    vec3 pos_b);

// Raycast v. Sphere
CollisionPoints
physics_collision_find_ray_sphere(Raycast *ray,
                                  SphereCollider *sphere,
                                  vec3 pos_sphere);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_PHYSICS_COLLISION