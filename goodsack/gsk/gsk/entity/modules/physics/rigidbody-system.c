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
#define USING_FRICTION         0
#define DEBUG_POINTS           3 // 0 -- OFF | value = entity id

typedef struct _SolverData
{
    struct ComponentRigidbody *const p_rigidbody;
    struct ComponentTransform *const p_transform;
    gsk_CollisionResult *p_collision_result;
    const gsk_Entity entity;
    const f64 delta;
} _SolverData;

static void
_position_solver(_SolverData solver_data);

static void
_impulse_solver(_SolverData solver_data);

//-----------------------------------------------------------------------------
static void
__debug_points(const _SolverData solver_data, u32 id)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_Entity entity                     = solver_data.entity;

#if 0
    if (entity.id == id) {
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               entity.id + 1,
                               collision_result->points.point_a,
                               (vec4) {0, 1, 0, 0},
                               FALSE);
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               entity.id + 2,
                               collision_result->points.point_b,
                               (vec4) {1, 0, 0, 0},
                               FALSE);
    }
#endif
}
//-----------------------------------------------------------------------------

static void
init(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_RIGIDBODY))) return;
    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;

    struct ComponentRigidbody *rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    struct ComponentCollider *collider   = gsk_ecs_get(entity, C_COLLIDER);

    glm_vec3_zero(rigidbody->force);
    glm_vec3_zero(rigidbody->linear_velocity);
    glm_vec3_zero(rigidbody->angular_velocity);

    // friction constants
    rigidbody->static_friction  = 0.6f;
    rigidbody->dynamic_friction = 0.3f;

    // calculate rotational inertia
    f32 inertia = 0;
    if (collider->type == COLLIDER_SPHERE) {

        // I = 2/5mr^2 -- solid sphere
        inertia = (2.0f / 5.0f) * rigidbody->mass *
                  ((gsk_SphereCollider *)collider->pCollider)->radius;
    }

    else if (collider->type == COLLIDER_BOX) {

        // TODO: get width/height from bounds
        f32 width  = 2;
        f32 height = 2;

        // I_d = 1/12m(w^2 + h^2) -- rectangular cuboid depth
        inertia =
          (1.0f / 12.0f) * rigidbody->mass * pow(width, 2) + pow(height, 2);
    }

    // inverse mass and inertia
    f32 inverse_mass    = 1.0f / rigidbody->mass;
    f32 inverse_inertia = 1.0f / inertia;

    // Initialize the physics solver
    rigidbody->solver                       = malloc(sizeof(gsk_PhysicsSolver));
    *(gsk_PhysicsSolver *)rigidbody->solver = gsk_physics_solver_init();
}

static void
fixed_update(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_RIGIDBODY))) return;
    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;
    if (!(gsk_ecs_has(entity, C_TRANSFORM))) return;

    struct ComponentRigidbody *rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    struct ComponentCollider *collider   = gsk_ecs_get(entity, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(entity, C_TRANSFORM);

    // Calculate simulation-time
    const gsk_Time time = gsk_device_getTime();
    const f64 delta     = time.fixed_delta_time * time.time_scale;

    // --
    // -- Check for solvers/collision results

    gsk_PhysicsSolver *pSolver = (gsk_PhysicsSolver *)rigidbody->solver;
    int total_solvers          = (int)pSolver->solvers_list->list_next;

    for (int i = 0; i < total_solvers; i++) {
        gsk_CollisionResult *pResult = &pSolver->solvers[i];

        // --
        // construct _SolverData used to pass into solver functions
        _SolverData solver_data = {.p_rigidbody        = rigidbody,
                                   .p_transform        = transform,
                                   .p_collision_result = pResult,
                                   .entity             = entity,
                                   .delta              = delta};

        // --
        // Run Solvers

        _position_solver(solver_data);
        _impulse_solver(solver_data);

#if DEBUG_POINTS
        __debug_points(solver_data, DEBUG_POINTS);
#endif // DEBUG_POINTS

        // Pop our solver
        gsk_physics_solver_pop((gsk_PhysicsSolver *)rigidbody->solver);
    }

    // --
    // -- Integrate velocities

    // position += velocity * delta_time;
    vec3 vD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->linear_velocity, delta, vD);
    glm_vec3_add(transform->position, vD, transform->position);

#if USING_ANGULAR_VELOCITY
    // orientation += angular_velocity * delta_time;
    vec3 aVD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->angular_velocity, delta, aVD);
    glm_vec3_add(transform->orientation, aVD, transform->orientation);
#endif

    // --
    // -- Reset net force

    glm_vec3_zero(rigidbody->force);
}
//-----------------------------------------------------------------------------
static void
_position_solver(_SolverData solver_data)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    struct ComponentTransform *transform  = solver_data.p_transform;

    gsk_DynamicBody body_a = collision_result->physics_mark.body_a;
    gsk_DynamicBody body_b = collision_result->physics_mark.body_b;

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(collision_result->points.normal, collision_normal);

    f32 inverse_scalar = body_a.inverse_mass + body_b.inverse_mass;

    const float percent = 0.8f;
    const float slop    = 0.01f;

    vec3 correction;
    f32 c_weight = fmax((collision_result->points.depth - slop), 0);
    glm_vec3_scale(collision_normal, percent, correction);
    glm_vec3_scale(correction, c_weight, correction);
    // glm_vec3_scale(correction, inverse_scalar, correction);

    // integrate new position
    glm_vec3_add(transform->position, correction, transform->position);
}
//-----------------------------------------------------------------------------
static void
_impulse_solver(_SolverData solver_data)
{

    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker               = collision_result->physics_mark;

    struct ComponentTransform *transform   = solver_data.p_transform;
    struct ComponentRigidbody *rigidbody_a = solver_data.p_rigidbody;
    const f64 delta                        = solver_data.delta;

    gsk_DynamicBody body_a = marker.body_a;
    gsk_DynamicBody body_b = marker.body_b;

    // --
    // Basic collision reflection

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(collision_result->points.normal, collision_normal);

    float restitution = 0.8f; // Bounce factor

    // calculate relative velocity
    float vDotN = (glm_vec3_dot(
      collision_result->physics_mark.relative_velocity, collision_normal));
    float F     = -(1.0f + restitution) * vDotN;
    F /= body_a.inverse_mass + body_b.inverse_mass;

    vec3 reflect = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(collision_normal, F, reflect);

    // prevent negative impulse
    if (vDotN >= 0.0f) { return; }

    // scale by inverse mass
    glm_vec3_scale(reflect, body_a.inverse_mass, reflect);

    // get point of contact
    vec3 ra, ra_perp;
    glm_vec3_subs(
      collision_result->points.point_a, collision_result->points.depth, ra);
    glm_vec3_sub(ra, transform->position, ra);

    // perpendicular
    glm_vec3_cross(transform->position, ra, ra_perp);

    if (solver_data.entity.id == DEBUG_POINTS) {
        gsk_debug_markers_push(solver_data.entity.ecs->renderer->debugContext,
                               solver_data.entity.id + 3,
                               ra_perp,
                               (vec4) {0, 1, 1, 1},
                               FALSE);
    }

    // angular velocity
    vec3 angular_linear_velocity_a;
    glm_vec3_mul(ra_perp, body_a.angular_velocity, angular_linear_velocity_a);

#if USING_FRICTION
    // --
    // Friction implementation

    vec3 tangent = GLM_VEC3_ZERO_INIT;

    // tangent = velocity - dot(velocity, normal) * normal;
    glm_vec3_scale(collision_normal,
                   glm_vec3_dot(marker.linear_velocity_a, collision_normal),
                   tangent);
    glm_vec3_sub(marker.linear_velocity_a, tangent, tangent);

    // check near-zero
    {
        float zerodist = glm_vec3_distance(tangent, GLM_VEC3_ZERO);
        // LOG_INFO("%f dist: ", zerodist);
        if (zerodist >= 0.001f) { glm_vec3_normalize(tangent); }
    }

    vec3 friction_impulse = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(reflect, friction_impulse);

    float sf = rigidbody_a->static_friction * rigidbody_a->mass;
    float df = rigidbody_a->dynamic_friction * rigidbody_a->mass;

    float j = -(collision_result->points.depth);

    if (abs(vDotN) <= j * sf) {
        // friction_impulse = -j * tangent * df
        glm_vec3_scale(tangent, -glm_dot(tangent, reflect), friction_impulse);
    } else {
        glm_vec3_scale(tangent, -j * df, friction_impulse);
    }
#endif // USING_FRICTION

#if USING_ANGULAR_VELOCITY
    // angular velocity
    //) Ft = -(velocity + (vDotN*collision_normal));
    vec3 Ft = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(collision_normal, vDotN, Ft);
    glm_vec3_add(Ft, rigidbody->linear_velocity, Ft);
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

    // --
    // Apply impulses

    glm_vec3_add(
      rigidbody_a->linear_velocity, reflect, rigidbody_a->linear_velocity);
    // glm_vec3_sub(rigidbody_a->linear_velocity,
    //             friction_impulse,
    //             rigidbody_a->linear_velocity);

#if USING_ANGULAR_VELOCITY
    glm_vec3_add(torque, Ft, rigidbody->angular_velocity);
#endif // USING_ANGULAR_VELOCITY
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
