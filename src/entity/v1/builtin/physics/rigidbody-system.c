#include "rigidbody-system.h"

#include <util/logger.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <core/device/device.h>

static void
init(Entity e)
{
    if (!(ecs_has(e, C_RIGIDBODY))) return;
    if (!(ecs_has(e, C_TRANSFORM))) return;

    struct ComponentRigidbody *rigidbody = ecs_get(e, C_RIGIDBODY);
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    glm_vec3_zero(rigidbody->force);
    glm_vec3_zero(rigidbody->velocity);
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_RIGIDBODY))) return;
    if (!(ecs_has(e, C_COLLIDER))) return;
    if (!(ecs_has(e, C_TRANSFORM))) return;

    struct ComponentRigidbody *rigidbody = ecs_get(e, C_RIGIDBODY);
    struct ComponentCollider *collider   = ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    // --
    // -- Add gravity to net force (mass considered)

    // mass * gravity
    vec3 mG = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->gravity, rigidbody->mass, mG);
    glm_vec3_add(rigidbody->force, mG, rigidbody->force);

    // TODO: check intersection here after gravity check
    if (collider->isColliding) {

        // --
        // Poor-position Solver

        vec3 newPos = {0, -0.101f, -1};
        glm_vec3_copy(newPos, transform->position);

        // --
        // Velocity/Impulse Solver

        vec3 plane_normal = {0, 1, 0};
        float restitution = 0.5f; // Bounce factor

        float vDotN = glm_vec3_dot(rigidbody->velocity, plane_normal);
        float F = -(1.0f + restitution) * vDotN;
        F *= rigidbody->mass;
        vec3 reflect = {0, F / rigidbody->mass, 0};
        glm_vec3_add(rigidbody->velocity, reflect, rigidbody->velocity);

#if 0
        float cutoff = 2; // stop velocity and force when F < this
        if(F < cutoff) {
            glm_vec3_zero(rigidbody->velocity);
            glm_vec3_zero(rigidbody->force);
        }
#endif
    }

    // --
    // -- Add net force to velocity (mass considered)

    // velocity += force / mass * delta;
    vec3 fDm = GLM_VEC3_ZERO_INIT;
    glm_vec3_divs(rigidbody->force, rigidbody->mass, fDm);
    glm_vec3_scale(fDm, device_getAnalytics().delta, fDm);
    glm_vec3_add(rigidbody->velocity, fDm, rigidbody->velocity);

    // --
    // -- Add velocity to position

    // position += velocity * delta;
    vec3 vD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->velocity, device_getAnalytics().delta, vD);
    glm_vec3_add(transform->position, vD, transform->position);

    // --
    // -- Reset net force

    glm_vec3_zero(rigidbody->force);
}

void
s_rigidbody_system_init(ECS *ecs)
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
