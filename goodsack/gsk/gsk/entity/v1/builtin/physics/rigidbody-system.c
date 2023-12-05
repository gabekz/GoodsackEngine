/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "rigidbody-system.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/device/device.h"
#include "physics/physics_solver.h"

static void
_position_solver(struct ComponentRigidbody *rigidbody,
                 struct ComponentTransform *transform,
                 gsk_CollisionResult *collision_result);

static void
_impulse_solver(struct ComponentRigidbody *rigidbody,
                struct ComponentTransform *transform,
                gsk_CollisionResult *collision_result);
static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_RIGIDBODY))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

    struct ComponentRigidbody *rigidbody = gsk_ecs_get(e, C_RIGIDBODY);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    glm_vec3_zero(rigidbody->force);
    glm_vec3_zero(rigidbody->velocity);
    glm_vec3_zero(rigidbody->angular_velocity);

    // Initialize the physics solver
    rigidbody->solver                       = malloc(sizeof(gsk_PhysicsSolver));
    *(gsk_PhysicsSolver *)rigidbody->solver = gsk_physics_solver_init();

    // LOG_INFO("solvers %u", ((PhysicsSolver
    // *)rigidbody->solver)->solvers_count);
}

static void
update(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_RIGIDBODY))) return;
    if (!(gsk_ecs_has(e, C_COLLIDER))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

    struct ComponentRigidbody *rigidbody = gsk_ecs_get(e, C_RIGIDBODY);
    struct ComponentCollider *collider   = gsk_ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    // --
    // -- Add gravity to net force (mass considered)

    // mass * gravity
    vec3 mG = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->gravity, rigidbody->mass, mG);
    glm_vec3_add(rigidbody->force, mG, rigidbody->force);

    // --
    // -- Check for solvers/collision results

    gsk_PhysicsSolver *pSolver = (gsk_PhysicsSolver *)rigidbody->solver;
    int total_solvers          = (int)pSolver->solver_next;

    for (int i = 0; i < total_solvers; i++) {
        // LOG_INFO("DO SOLVER");

        gsk_CollisionResult *pResult = &pSolver->solvers[i];

        vec3 collision_normal = GLM_VEC3_ZERO_INIT;
        glm_vec3_copy(pResult->points.normal, collision_normal);

        // --
        // Run Solvers

        _position_solver(rigidbody, transform, pResult);
        _impulse_solver(rigidbody, transform, pResult);

        // Pop our solver
        gsk_physics_solver_pop((gsk_PhysicsSolver *)rigidbody->solver);
    }

    // --
    // -- Add net force to velocity (mass considered)

    // velocity += force / mass * delta;
    vec3 fDm = GLM_VEC3_ZERO_INIT;
    glm_vec3_divs(rigidbody->force, rigidbody->mass, fDm);
    glm_vec3_scale(fDm, gsk_device_getAnalytics().delta, fDm);
    glm_vec3_add(rigidbody->velocity, fDm, rigidbody->velocity);

    // --
    // -- Add velocity to position

    // position += velocity * delta;
    vec3 vD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->velocity, gsk_device_getAnalytics().delta, vD);
    glm_vec3_add(transform->position, vD, transform->position);

    // orientation += angular_velocity * delta;
    vec3 aVD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(
      rigidbody->angular_velocity, gsk_device_getAnalytics().delta, aVD);
    glm_vec3_add(transform->orientation, aVD, transform->orientation);

    // --
    // -- Reset net force

    glm_vec3_zero(rigidbody->force);
}

static void
_position_solver(struct ComponentRigidbody *rigidbody,
                 struct ComponentTransform *transform,
                 gsk_CollisionResult *collision_result)
{
// velocity
#define USE_VELOCITY       0
#define USE_VELOCITY_DELTA 0

// raw position
#define AFFECT_POS_DIRECT 1
#define USE_POS_DELTA     0

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(collision_result->points.normal, collision_normal);

    const float percent = 0.2f;
    const float slop    = 0.01f;

    vec3 correction = {0, 0, 0};
    glm_vec3_scale(collision_normal,
                   percent *
                     fmax((collision_result->points.depth - slop), 0.0f),
                   correction);
    glm_vec3_negate(correction);

    // LOG_INFO("correction: %f\t%f\t%f", correction[0], correction[1],
    // correction[2]);

#if USE_VELOCITY
#if USE_VELOCITY_DELTA
    glm_vec3_scale(correction,
                   gsk_device_getAnalytics().delta * rigidbody->mass,
                   correction);
#endif // USE_DELTA
    // NOTE: This one is pretty interseting..
    glm_vec3_sub(rigidbody->velocity, correction, rigidbody->velocity);
#endif // USE_VELOCITY

#if AFFECT_POS_DIRECT
#if USE_POS_DELTA
    glm_vec3_scale(correction,
                   gsk_device_getAnalytics().delta * rigidbody->mass,
                   correction);
#endif
    glm_vec3_sub(transform->position, correction, transform->position);
#endif // AFFECT_POS_DIRECT
}

static void
_impulse_solver(struct ComponentRigidbody *rigidbody,
                struct ComponentTransform *transform,
                gsk_CollisionResult *collision_result)
{

#define USE_CUTOFF   0 // a.k.a. FAKE friction
#define USE_FRICTION 1 // a.k.a. angular velocity

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(collision_result->points.normal, collision_normal);

    float restitution = 0.8f; // Bounce factor

    float vDotN = glm_vec3_dot(rigidbody->velocity, collision_normal);
    float F     = -(1.0f + restitution) * vDotN;
    F *= rigidbody->mass;

    // vec3 reflect = {0, F / rigidbody->mass, 0};
    vec3 reflect = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(collision_normal, F / rigidbody->mass, reflect);

    // LOG_INFO("%f", F);
    //  cutoff for impulse solver. Can potentially fix issues with
    //  double-precision.
#if USE_CUTOFF
    float cutoff = 2; // stop velocity and force when F < this
    if (F < cutoff && F > -cutoff) {
        glm_vec3_zero(rigidbody->angular_velocity);
        glm_vec3_zero(rigidbody->velocity);
        glm_vec3_zero(rigidbody->force);
    }
#endif // USE_CUTOFF

    // Angular Velocity / Friction
#if USE_FRICTION
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
    glm_vec3_sub(transform->position, collision_normal, friction_contact);

    //) Torque = cross(vec2Centre, Ft)
    vec3 torque = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(friction_contact, Ft, torque);

    //)
    // Ball.AngularAccelerate(Torque/Ball.getMomentofIntertia(normalize(torque)))
    // inertia is I = 2/5mr^2 for solid sphere

    float I = 0.4f * rigidbody->mass * 0.2f * 0.2f;
    glm_vec3_divs(torque, I, torque);
    // glm_vec3_normalize(torque);

    glm_vec3_copy(torque, rigidbody->angular_velocity);
    // LOG_INFO("%f\t%f\t%f", torque[0], torque[1], torque[2]);
#endif // USE_FRICTION

    // glm_vec3_normalize(Ft);
    // glm_vec3_mul(reflect, Ft, reflect);

    // glm_vec3_scale(reflect,500, reflect);
    // glm_vec3_add(rigidbody->force, reflect, rigidbody->force);

    glm_vec3_add(rigidbody->velocity, reflect, rigidbody->velocity);
}

void
s_rigidbody_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init        = (gsk_ECSSubscriber)init,
                              .destroy     = NULL,
                              .render      = NULL,
                              .update      = (gsk_ECSSubscriber)update,
                              .late_update = NULL,
                            }));
}
