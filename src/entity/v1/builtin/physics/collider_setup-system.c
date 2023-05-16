
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
        sphereCollider->radius         = 1;

        ((Collider *)collider->pCollider)->collider_data =
          (SphereCollider *)sphereCollider;

    } else if (collider->type == 2) {
        PlaneCollider *planeCollider = malloc(sizeof(PlaneCollider));
        planeCollider->distance      = 0.0175f;

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

    for (int i = 0; i < e.ecs->nextId; i++) {
        if (e.index == i) continue; // do not check self

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

        CollisionPoints points;
        // determine which collision-test function to use
        if (collider->type == 1 && compareCollider->type == 2) {
            // sphere v. plane
            points = physics_collision_find_sphere_plane(
              ((Collider *)collider->pCollider)->collider_data,
              ((Collider *)compareCollider->pCollider)->collider_data,
              transform->position,
              compareTransform->position);

        } else if (collider->type == 2 && compareCollider->type == 1) {
            // plane v. sphere
            points =
              physics_collision_find_plane_sphere(collider->pCollider,
                                                  compareCollider->pCollider,
                                                  transform->position,
                                                  compareTransform->position);
        }

        // Collision points
        if (points.has_collision) { LOG_INFO("Collision!"); }
        collider->isColliding = points.has_collision;
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
