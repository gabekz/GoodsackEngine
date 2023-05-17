#ifndef H_PHYSICS_COLLISION
#define H_PHYSICS_COLLISION

#include <util/maths.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct CollisionPoints_t
{
    vec3 point_a;       // furthest point of A into B
    vec3 point_b;       // furthest point of B into A
    vec3 normal;        // B - A normalized
    float depth;        // Length of B - A
    ui16 has_collision; // bool
} CollisionPoints;

typedef struct CollisionResult_t
{
    void *object_a;
    void *object_b;
    CollisionPoints points;
} CollisionResult;

typedef struct SphereCollider_t
{
    vec3 center;
    float radius;
} SphereCollider;

typedef struct PlaneCollider_t
{
    vec3 plane;
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

typedef struct Collider_t
{
    void *collider_data;
    ui16 collider_data_type;

    vec3 position;

    ui16 is_dynamic, is_trigger;
} Collider;

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