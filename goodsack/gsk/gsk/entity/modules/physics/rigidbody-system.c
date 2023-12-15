/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "rigidbody-system.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "tools/debug/debug_context.h"

#include "core/device/device.h"
#include "physics/physics_solver.h"

#define USING_ANGULAR_VELOCITY 0

static void
_position_solver(struct ComponentRigidbody *rigidbody,
                 struct ComponentTransform *transform,
                 gsk_CollisionResult *collision_result,
                 gsk_Entity entity);

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
    // struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    glm_vec3_zero(rigidbody->force);
    glm_vec3_zero(rigidbody->velocity);
    glm_vec3_zero(rigidbody->angular_velocity);

    rigidbody->static_friction  = 0.6f;
    rigidbody->dynamic_friction = 0.3f;

    // Initialize the physics solver
    rigidbody->solver                       = malloc(sizeof(gsk_PhysicsSolver));
    *(gsk_PhysicsSolver *)rigidbody->solver = gsk_physics_solver_init();
}

static void
fixed_update(gsk_Entity e)
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
    int total_solvers          = (int)pSolver->solvers_list->list_next;

    // Calculate simulation-time
    const gsk_Time time = gsk_device_getTime();
    const f64 delta     = time.fixed_delta_time * time.time_scale;

    for (int i = 0; i < total_solvers; i++) {

        gsk_CollisionResult *pResult = &pSolver->solvers[i];

        // --
        // Run Solvers

        _position_solver(rigidbody, transform, pResult, e);
        _impulse_solver(rigidbody, transform, pResult);

#if 1
        if (e.id == 4) {

            // --
            // push debug marker
            gsk_debug_markers_push(e.ecs->renderer->debugContext,
                                   e.id + 1,
                                   pResult->points.point_a,
                                   (vec4) {0, 1, 0, 0},
                                   FALSE);
            gsk_debug_markers_push(e.ecs->renderer->debugContext,
                                   e.id + 2,
                                   pResult->points.point_b,
                                   (vec4) {1, 0, 0, 0},
                                   FALSE);
        }
#endif

        // Pop our solver
        gsk_physics_solver_pop((gsk_PhysicsSolver *)rigidbody->solver);
    }

    // --
    // -- Add net force to velocity (mass considered)

    // velocity += force / mass * delta_time;
    vec3 fDm = GLM_VEC3_ZERO_INIT;
    glm_vec3_divs(rigidbody->force, rigidbody->mass, fDm);
    glm_vec3_scale(fDm, delta, fDm);
    glm_vec3_add(rigidbody->velocity, fDm, rigidbody->velocity);

    // --
    // -- Add velocity to position

    // velocity += angular_velocity
    // glm_vec3_add(
    //  rigidbody->angular_velocity, rigidbody->velocity, rigidbody->velocity);

    // orientation += angular_velocity * delta_time;
    vec3 aVD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->angular_velocity, delta, aVD);
    glm_vec3_add(transform->orientation, aVD, transform->orientation);

// velocity += normalized angular velocity + velocity
#if 0
    vec3 aVV = GLM_VEC3_ZERO_INIT;
    glm_vec3_normalize_to(rigidbody->angular_velocity, aVV);
    glm_vec3_scale(aVV, delta, aVV);
    glm_vec3_divs(aVV, rigidbody->mass, aVV);
    glm_vec3_add(aVV, rigidbody->velocity, rigidbody->velocity);
#endif

    // position += velocity * delta_time;
    vec3 vD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->velocity, delta, vD);
    glm_vec3_add(transform->position, vD, transform->position);

    // --
    // -- Reset net force

    glm_vec3_zero(rigidbody->force);
}
//-----------------------------------------------------------------------------
static void
_position_solver(struct ComponentRigidbody *rigidbody,
                 struct ComponentTransform *transform,
                 gsk_CollisionResult *collision_result,
                 gsk_Entity entity)
{
// velocity
#define USE_VELOCITY       0
#define USE_VELOCITY_DELTA 0

// raw position
#define AFFECT_POS_DIRECT 1
#define USE_POS_DELTA     0

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(collision_result->points.normal, collision_normal);

    const float percent = 0.10f;
    const float slop    = 0.005f;

    vec3 correction = {0, 0, 0};
    glm_vec3_scale(
      collision_normal,
      fmax((collision_result->points.depth + slop) * percent, 0.0f),
      correction);
    glm_vec3_negate(correction);

    // LOG_INFO("correction: %f\t%f\t%f", correction[0], correction[1],
    // correction[2]);

#if USE_VELOCITY
#if USE_VELOCITY_DELTA
    glm_vec3_scale(correction,
                   gsk_device_getTime().delta_time * rigidbody->mass,
                   correction);
#endif // USE_DELTA
    // NOTE: This one is pretty interseting..
    glm_vec3_sub(rigidbody->velocity, correction, rigidbody->velocity);
#endif // USE_VELOCITY

#if AFFECT_POS_DIRECT
#if USE_POS_DELTA
    glm_vec3_scale(correction,
                   gsk_device_getTime().delta_time * rigidbody->mass,
                   correction);
#endif
    glm_vec3_sub(transform->position, correction, transform->position);

#endif // AFFECT_POS_DIRECT
}
//-----------------------------------------------------------------------------
static void
_impulse_solver(struct ComponentRigidbody *rigidbody,
                struct ComponentTransform *transform,
                gsk_CollisionResult *collision_result)
{

    // Calculate simulation-time
    const gsk_Time time = gsk_device_getTime();
    const f64 delta     = time.fixed_delta_time * time.time_scale;

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(collision_result->points.normal, collision_normal);

    vec3 tangent = GLM_VEC3_ZERO_INIT;

    // tangent = velocity - dot(velocity, normal) * normal;
    glm_vec3_scale(collision_normal,
                   glm_vec3_dot(rigidbody->velocity, collision_normal),
                   tangent);
    glm_vec3_sub(rigidbody->velocity, tangent, tangent);

    // check near-zero
    {
        float zerodist = glm_vec3_distance(tangent, GLM_VEC3_ZERO);
        // LOG_INFO("%f dist: ", zerodist);
        if (zerodist >= 0.001f) { glm_vec3_normalize(tangent); }
    }

    // LOG_INFO("%f\t%f\t%f", tangent[0], tangent[1], tangent[2]);

    // float jt = -(glm_vec3_dot(rigidbody->velocity, collision_normal));

    float restitution = 0.8f; // Bounce factor

    float vDotN = (glm_vec3_dot(rigidbody->velocity, collision_normal));
    float F     = -(1.0f + restitution) * vDotN;
    F *= rigidbody->mass;

    // vec3 reflect = {0, F / rigidbody->mass, 0};
    vec3 reflect = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(collision_normal, F / rigidbody->mass, reflect);
    // #endif

    vec3 friction_impulse = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(reflect, friction_impulse);

    float sf = rigidbody->static_friction * rigidbody->mass;
    float df = rigidbody->dynamic_friction * rigidbody->mass;

    float j = -(collision_result->points.depth);

    if (abs(vDotN) <= j * sf) {
        // friction_impulse = -j * tangent * df
        glm_vec3_mul(tangent, reflect, friction_impulse);
    } else {
        glm_vec3_scale(tangent, -j * df, friction_impulse);
    }

#if USING_ANGULAR_VELOCITY
    // angular velocity
    //) Ft = -(velocity + (vDotN*collision_normal));
    vec3 Ft = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(collision_normal, vDotN, Ft);
    glm_vec3_add(Ft, rigidbody->velocity, Ft);
    // glm_vec3_negate(Ft);

    //) Ft *= A.kineticFriction + B.kineticFriction
    // TODO: for now, kinectic friction is coefficient magic
    glm_vec3_scale(Ft, 0.57f, Ft); // Steel on Steel?
    //) Ft *= mass
    glm_vec3_scale(Ft, rigidbody->mass, Ft);

    //) Torque = cross(vec2Centre, Ft)
    vec3 torque = GLM_VEC3_ZERO_INIT;
    glm_vec3_cross(tangent, Ft, torque);

    // inertia is I = 2/5mr^2 for solid sphere

    float I = 0.4f * rigidbody->mass * 0.2f * 0.2f;
    glm_vec3_divs(torque, I, torque);
#endif // USING_ANGULAR_VELOCITY

    // Apply impulses
    glm_vec3_add(rigidbody->velocity, reflect, rigidbody->velocity);
#if USING_ANGULAR_VELOCITY
    glm_vec3_add(torque, Ft, rigidbody->angular_velocity);
#endif // USING_ANGULAR_VELOCITY
    glm_vec3_sub(rigidbody->velocity, friction_impulse, rigidbody->velocity);
}

void
s_rigidbody_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init         = (gsk_ECSSubscriber)init,
                              .destroy      = NULL,
                              .render       = NULL,
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                              .update       = NULL,
                              .late_update  = NULL,
                            }));
}
