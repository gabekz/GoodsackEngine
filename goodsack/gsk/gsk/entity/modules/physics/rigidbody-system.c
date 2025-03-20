/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Move solvers to separate file

#include "rigidbody-system.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"
#include "util/vec_colors.h"

#include "tools/debug/debug_context.h"
#include "tools/debug/debug_draw_line.h"

#include "core/device/device.h"
#include "physics/physics_solver.h"

#include "entity/modules/physics/solvers/friction_solver.h"

// Functionality toggles
#define DEBUG_TRACK  1
#define DEBUG_POINTS 0 // 0 -- OFF | value = entity id

#define CALC_INERTIA 1

// physics default values
#define DEFAULT_RESTITUION       0.5f
#define DEFAULT_STATIC_FRICTION  0.6f
#define DEFAULT_DYNAMIC_FRICTION 0.4f

// constant for putting dynamic objects to sleep
#define SLEEP_EPSILON 0.5f

static void
_position_solver(_SolverData solver_data);

//-----------------------------------------------------------------------------
#if DEBUG_TRACK
static u32 s_dbg_instance       = 0xFF;
static u32 s_dbg_instance_begin = 0xFF;

static void
__debug_ray(const _SolverData solver_data,
            vec3 start,
            vec3 direction,
            vec4 color)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_Entity entity                     = solver_data.entity;
    if (DEBUG_POINTS && entity.id == DEBUG_POINTS)
    {
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_RAY,
                               s_dbg_instance,
                               start,
                               direction,
                               1,
                               color,
                               FALSE);
        s_dbg_instance++;
    }
}
#endif

//-----------------------------------------------------------------------------
static void
__debug_points(const _SolverData solver_data)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_Entity entity                     = solver_data.entity;

    if (DEBUG_POINTS && entity.id == DEBUG_POINTS)
    {

        // collision normal
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_RAY,
                               entity.id + 25,
                               solver_data.p_transform->position,
                               collision_result->points.normal,
                               5,
                               VCOL_GREEN,
                               FALSE);

        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_POINT,
                               entity.id + 26,
                               collision_result->points.point_a,
                               collision_result->points.normal,
                               5,
                               VCOL_GREEN,
                               FALSE);

        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_POINT,
                               entity.id + 27,
                               collision_result->points.point_b,
                               collision_result->points.normal,
                               5,
                               VCOL_RED,
                               FALSE);
    }
}
//-----------------------------------------------------------------------------

static void
init(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_RIGIDBODY))) return;
    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;

    struct ComponentRigidbody *rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    struct ComponentCollider *collider   = gsk_ecs_get(entity, C_COLLIDER);

#if 0
    glm_vec3_zero(rigidbody->force_impulse);
    glm_vec3_zero(rigidbody->force_velocity);

    glm_vec3_zero(rigidbody->torque);
#endif

    glm_vec3_zero(rigidbody->linear_velocity);
    glm_vec3_zero(rigidbody->angular_velocity);

    // friction constants
    // todo: add restitution here
    rigidbody->static_friction  = DEFAULT_STATIC_FRICTION;
    rigidbody->dynamic_friction = DEFAULT_DYNAMIC_FRICTION;

    if (rigidbody->mass <= 0)
    {
        LOG_WARN("mass was <= 0 - setting mass to 1");
        rigidbody->mass = 1.0f;
    }

#if CALC_INERTIA
    // calculate rotational inertia
    // TODO: Defaults
    f32 inertia = 0;
    if (collider->type == COLLIDER_SPHERE)
    {

        // I = 2/5mr^2 -- solid sphere
        inertia = (2.0f / 5.0f) * rigidbody->mass *
                  pow(((gsk_SphereCollider *)collider->pCollider)->radius, 2);
    }

    else if (collider->type == COLLIDER_BOX)
    {

        // TODO: get width/height from bounds
        f32 width  = 1;
        f32 height = 1;

        // I_d = 1/12m(w^2 + h^2) -- rectangular cuboid depth
        inertia =
          (1.0f / 12.0f) * rigidbody->mass * pow(width, 2) + pow(height, 2);
    }

    // inverse mass and inertia
    rigidbody->inverse_mass    = 1.0f / rigidbody->mass;
    rigidbody->inverse_inertia = 1.0f / inertia;
#endif

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

    if (rigidbody->is_kinematic == TRUE) { return; }

    // Calculate simulation-time
    const gsk_Time time = gsk_device_getTime();
    const f64 delta     = time.fixed_delta_time * time.time_scale;

#if 0
    // Add Gravitational force

    glm_vec3_scale(
      rigidbody->gravity, rigidbody->mass, rigidbody->force_impulse);

    // add impulse force
    vec3 fDm = GLM_VEC3_ZERO_INIT;
    glm_vec3_divs(rigidbody->force_impulse, rigidbody->mass, fDm);
    // glm_vec3_scale(fDm, delta, fDm);
    glm_vec3_add(fDm, rigidbody->force_impulse, rigidbody->force_impulse);

    glm_vec3_add(rigidbody->force_impulse,
                 rigidbody->linear_velocity,
                 rigidbody->linear_velocity);
#endif

    // --
    // -- Check for solvers/collision results

    gsk_PhysicsSolver *pSolver = (gsk_PhysicsSolver *)rigidbody->solver;
    int total_solvers          = (int)pSolver->solvers_list->list_next;

#if DEBUG_TRACK
    s_dbg_instance = s_dbg_instance_begin;
#endif

    for (int i = 0; i < total_solvers; i++)
    {
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
        impulse_solver_with_rotation_friction(solver_data);

        // --
        // Run debug markers

#if DEBUG_POINTS
        __debug_points(solver_data);
#endif // DEBUG_POINTS

        // --
        // Pop our solver

        gsk_physics_solver_pop((gsk_PhysicsSolver *)rigidbody->solver);
    }

    if (total_solvers > 0)
    {
        glm_vec3_divs(
          rigidbody->force_velocity, total_solvers, rigidbody->force_velocity);
        glm_vec3_divs(rigidbody->torque, total_solvers, rigidbody->torque);
    }

    // --
    // -- Add force to linear velocity (ignore mass)
    glm_vec3_add(rigidbody->linear_velocity,
                 rigidbody->force_velocity,
                 rigidbody->linear_velocity);

    // Rigidbody sleep threshold
    if (glm_vec3_norm(rigidbody->linear_velocity) <= SLEEP_EPSILON)
    {
        glm_vec3_zero(rigidbody->force_impulse);
        glm_vec3_zero(rigidbody->force_velocity);
        glm_vec3_zero(rigidbody->torque);

        glm_vec3_zero(rigidbody->angular_velocity);
        // glm_vec3_zero(rigidbody->linear_velocity);
        return;
    }

    // --
    // -- Integrate velocities

    // calculate : position += velocity * delta_time;
    vec3 vD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->linear_velocity, delta, vD);

    // update position
    glm_vec3_add(transform->position, vD, transform->position);

#if 0
    // calculate : orientation += angular_velocity * delta_time;
    vec3 aVD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->angular_velocity, delta, aVD);

    aVD[0] = glm_deg(aVD[0]);
    aVD[1] = glm_deg(aVD[1]);
    aVD[2] = glm_deg(aVD[2]);

    // update orientation
    glm_vec3_add(transform->orientation, aVD, transform->orientation);
#else

    float rollingFriction = 0.001f;
    vec3 frictionTorque2;

    glm_vec3_copy(rigidbody->angular_velocity, frictionTorque2);
    glm_vec3_normalize(frictionTorque2); // direction opposite of spin
    glm_vec3_scale(frictionTorque2, -rollingFriction, frictionTorque2);
    glm_vec3_add(rigidbody->torque, frictionTorque2, rigidbody->torque);

    vec3 angularAccel;
    glm_vec3_copy(rigidbody->torque, angularAccel);

    glm_vec3_add(
      angularAccel, rigidbody->angular_velocity, rigidbody->angular_velocity);

    vec3 angularDeg; // angularDeg
    glm_vec3_scale(rigidbody->angular_velocity, delta, angularDeg);
    angularDeg[0] = glm_deg(angularDeg[0]);
    angularDeg[1] = glm_deg(angularDeg[1]);
    angularDeg[2] = glm_deg(angularDeg[2]);

    // update orientation
    glm_vec3_add(transform->orientation, angularDeg, transform->orientation);

#endif // orientation

    // --
    // -- Reset net forces

    glm_vec3_zero(rigidbody->force_velocity);
    glm_vec3_zero(rigidbody->force_impulse);
    glm_vec3_zero(rigidbody->torque);
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

#if 1
    // I think these are better settings right now..
    const float percent = 1.0f;
    const float slop    = 0.005f;
#else
    const float percent = 0.8f;
    const float slop    = 0.1f;
#endif

    vec3 correction;
    f32 c_weight = fmax((collision_result->points.depth - slop), 0);
    glm_vec3_scale(collision_normal, percent, correction);
    glm_vec3_scale(correction, c_weight, correction);

    // integrate new position
    glm_vec3_add(transform->position, correction, transform->position);
}
//-----------------------------------------------------------------------------
void
s_rigidbody_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init         = (gsk_ECSSubscriber)init,
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                            }));
}
