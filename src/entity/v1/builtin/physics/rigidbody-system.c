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

    // mass * gravity
    vec3 mG = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->gravity, rigidbody->mass, mG);
    glm_vec3_add(rigidbody->force, mG, rigidbody->force);

    // TODO: check intersection here after gravity check

    if (collider->isColliding) {
        // Horrendous negative-impulse force
        LOG_INFO("Horrendous?");

        vec3 p = {0, 80, 0};

        vec3 negVelocity = GLM_VEC3_ZERO_INIT;
        glm_vec3_negate_to(rigidbody->velocity, negVelocity);
        glm_vec3_abs(negVelocity, negVelocity);
        glm_vec3_mul(p, negVelocity, p);

        glm_vec3_zero(rigidbody->velocity);
        glm_vec3_zero(rigidbody->force);

        glm_vec3_add(rigidbody->force, p, rigidbody->force);
    };

    // velocity += force / mass * delta;
    vec3 fDm = GLM_VEC3_ZERO_INIT;
    glm_vec3_divs(rigidbody->force, rigidbody->mass, fDm);
    glm_vec3_scale(fDm, device_getAnalytics().delta, fDm);
    glm_vec3_add(rigidbody->velocity, fDm, rigidbody->velocity);

    // position += velocity * delta;
    vec3 vD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->velocity, device_getAnalytics().delta, vD);
    glm_vec3_add(transform->position, vD, transform->position);

    // reset net force
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
