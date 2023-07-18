
#include "collider_setup-system.h"

#include <util/maths.h>
#include <util/sysdefs.h>

#include <core/device/device.h>

#include <physics/physics_collision.h>

static void
init(Entity e)
{
    if (!(ecs_has(e, C_COLLIDER))) return;
    if (!(ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCollider *collider   = ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    collider->isColliding = false; // TODO: remove <- this is for testing

    collider->pCollider = (Collider *)malloc(sizeof(Collider));
    ((Collider *)collider->pCollider)->collider_data_type = collider->type;

    //(Collider *)(collider->pCollider).position = transform->position;

    // TODO: collider types
    if (collider->type == 1) {
        SphereCollider *sphereCollider = malloc(sizeof(SphereCollider));
        sphereCollider->radius         = .20f;
        // glm_vec3_copy(transform->position, sphereCollider->center);
        // sphereCollider.center = transform.position;

        ((Collider *)collider->pCollider)->collider_data =
          (SphereCollider *)sphereCollider;

    } else if (collider->type == 2) {
        PlaneCollider *planeCollider = malloc(sizeof(PlaneCollider));
        // planeCollider->distance      = 0.0175f;
        planeCollider->distance = 10;
        glm_vec3_zero(planeCollider->plane);
        glm_vec3_zero(planeCollider->normal);
        glm_vec3_copy(transform->position, planeCollider->plane);
        LOG_INFO("%f\t%f\t%f",
                 planeCollider->plane[0],
                 planeCollider->plane[1],
                 planeCollider->plane[2]);
        // planeCollider->plane         = transform->position;

        glm_vec3_normalize_to(planeCollider->plane, planeCollider->normal);

        LOG_INFO("%f\t%f\t%f",
                 planeCollider->normal[0],
                 planeCollider->normal[1],
                 planeCollider->normal[2]);

        ((Collider *)collider->pCollider)->collider_data =
          (PlaneCollider *)planeCollider;
    }
}

static void
update(Entity e)
{
    // test for collisions
    if (!(ecs_has(e, C_COLLIDER))) return;
    if (!(ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCollider *collider   = ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    collider->isColliding = 0;

    for (int i = 0; i < e.ecs->nextIndex; i++) {
        if (e.index == (EntityId)i) continue; // do not check self

        // TODO: fix look-up
        Entity e_compare = {
          .id    = (EntityId)i + 1,
          .index = (EntityId)i,
          .ecs   = e.ecs,
        };

        if (!ecs_has(e_compare, C_COLLIDER)) continue;
        if (!ecs_has(e_compare, C_TRANSFORM)) continue;

        struct ComponentCollider *compareCollider =
          ecs_get(e_compare, C_COLLIDER);

        struct ComponentTransform *compareTransform =
          ecs_get(e_compare, C_TRANSFORM);

        CollisionPoints points = {.has_collision = 0};

        //
        // determine which collision-test function to use
        //
        // sphere v. plane
        if (collider->type == 1 && compareCollider->type == 2) {
            points = physics_collision_find_sphere_plane(
              ((Collider *)collider->pCollider)->collider_data,
              ((Collider *)compareCollider->pCollider)->collider_data,
              transform->position,
              compareTransform->position);
        }

        // sphere v. sphere
#if 0
        else if (collider->type == 1 && compareCollider->type == 1) {
            points = physics_collision_find_sphere_sphere(
              ((Collider *)collider->pCollider)->collider_data,
              ((Collider *)compareCollider->pCollider)->collider_data,
              transform->position,
              compareTransform->position);
        }
#endif

        // plane v. sphere
        else if (collider->type == 2 && compareCollider->type == 1) {
            points =
              physics_collision_find_plane_sphere(collider->pCollider,
                                                  compareCollider->pCollider,
                                                  transform->position,
                                                  compareTransform->position);
        }

#if 0
            // TESTING RAY INTERSECT
            Raycast ray = {
              .origin    = {0.0f, 0.0f, 0.0f},
              .direction = {-1.0f, 1.0f, 0.0f},
            };
            CollisionPoints rayTest = physics_collision_find_ray_sphere(
              &ray,
              ((Collider *)collider->pCollider)->collider_data,
              transform->position);

            if (rayTest.has_collision) LOG_INFO("RAY SUCCESS");
#endif


        // Collision points
        if (points.has_collision) {
#if 0
            LOG_INFO("Collision!");
#endif
            collider->isColliding = points.has_collision;
            break; // TODO: should not break.
                   //- instead, get a collection of points
        }
    }
}

void
s_collider_setup_system_init(ECS *ecs)
{
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init        = (ECSSubscriber)init,
                          .destroy     = NULL,
                          .render      = NULL,
                          .update      = (ECSSubscriber)update,
                          .late_update = NULL,
                        }));
}
