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

        vec3 collision_normal = {0, 1, 0};
        float restitution = 0.5f; // Bounce factor

        float vDotN = glm_vec3_dot(rigidbody->velocity, collision_normal);
        float F = -(1.0f + restitution) * vDotN;
        F *= rigidbody->mass;
        vec3 reflect = {0, F / rigidbody->mass, 0};
        glm_vec3_add(rigidbody->velocity, reflect, rigidbody->velocity);

        // cutoff for impulse solver. Can potentially fix issues with
        // double-precision.
#if 1
        float cutoff = 2; // stop velocity and force when F < this
        if(F < cutoff) {
            glm_vec3_zero(rigidbody->velocity);
            glm_vec3_zero(rigidbody->force);
        }
#endif

        // Angular Velocity / Friction
#if 0

        //) Ft = -(velocity + (vDotN*collision_normal));
        vec3 Ft = GLM_VEC3_ZERO_INIT;
        glm_vec3_scale(collision_normal, vDotN, Ft);
        glm_vec3_add(Ft, rigidbody->velocity, Ft);
        glm_vec3_negate(Ft);
        
        //) Ft *= A.kineticFriction + B.kineticFriction
        // TODO: for now, kinectic friction is coefficient magic
        glm_vec3_scale(Ft, 0.57f, Ft); // Steel on Steel?
        //) Ft *= mass
        glm_vec3_scale(Ft, rigidbody->mass, Ft);

        //) vec2Centre = pos_a - contactPoint
        vec3 friction_contact = GLM_VEC3_ZERO_INIT;
        glm_vec3_sub(transform->position, (vec3){0, -0.3f, 0}, friction_contact);

        //) Torque = cross(vec2Centre, Ft)
        vec3 torque = GLM_VEC3_ZERO_INIT;
        glm_vec3_cross(friction_contact, Ft, torque);

        //) Ball.AngularAccelerate(Torque/Ball.getMomentofIntertia(normalize(torque)))
        // inertia is I = 2/5mr^2 for solid sphere

        float I = 0.4f * rigidbody->mass * 0.2f * 0.2f;
        glm_vec3_divs(torque, I, torque);
        //glm_vec3_normalize(torque);

        glm_vec3_copy(torque, transform->orientation);

        LOG_INFO("%f\t%f\t%f", torque[0], torque[1], torque[2]);

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
