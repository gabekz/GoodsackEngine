/*
 * Copyright (c) 2023, Gabriel Kutuzov
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
#define DEBUG_POINTS   3 // 0 -- OFF | value = entity id
#define DEBUG_TRACK 0

#define CALC_INERTIA 0

// physics default values
#define DEFAULT_RESTITUION       0.5f
#define DEFAULT_STATIC_FRICTION  0.6f
#define DEFAULT_DYNAMIC_FRICTION 0.4f

static void
_position_solver(_SolverData solver_data);

static void
_impulse_solver_with_rotation(_SolverData solver_data);

static void
_impulse_solver_with_rotation_friction(_SolverData solver_data);

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
    if (DEBUG_POINTS && entity.id == DEBUG_POINTS) {
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
__debug_points(const _SolverData solver_data, u32 id)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_Entity entity                     = solver_data.entity;

    if (entity.id == id) {

        // collision normal
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_RAY,
                               entity.id + 2,
                               solver_data.p_transform->position,
                               collision_result->points.normal,
                               5,
                               VCOL_GREEN,
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

    glm_vec3_zero(rigidbody->force);
    glm_vec3_zero(rigidbody->linear_velocity);
    glm_vec3_zero(rigidbody->angular_velocity);

    // friction constants
    // todo: add restitution here
    rigidbody->static_friction  = DEFAULT_STATIC_FRICTION;
    rigidbody->dynamic_friction = DEFAULT_DYNAMIC_FRICTION;

#if CALC_INERTIA
    // calculate rotational inertia
    // TODO: Defaults
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

    // Calculate simulation-time
    const gsk_Time time = gsk_device_getTime();
    const f64 delta     = time.fixed_delta_time * time.time_scale;

    // --
    // -- Check for solvers/collision results

    gsk_PhysicsSolver *pSolver = (gsk_PhysicsSolver *)rigidbody->solver;
    int total_solvers          = (int)pSolver->solvers_list->list_next;

#if DEBUG_TRACK
    s_dbg_instance = s_dbg_instance_begin;
#endif

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
        impulse_solver_with_rotation_friction(solver_data);
        // --
        // Run debug markers

#if DEBUG_POINTS
        __debug_points(solver_data, DEBUG_POINTS);
#endif // DEBUG_POINTS

        // --
        // Pop our solver

        gsk_physics_solver_pop((gsk_PhysicsSolver *)rigidbody->solver);
    }

    // --
    // -- Integrate velocities

    // position += velocity * delta_time;
    vec3 vD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->linear_velocity, delta, vD);
    glm_vec3_add(transform->position, vD, transform->position);

#if 1
    // orientation += angular_velocity * delta_time;
    vec3 aVD = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->angular_velocity, delta, aVD);

#if 1
    aVD[0] = glm_deg(aVD[0]);
    aVD[1] = glm_deg(aVD[1]);
    aVD[2] = glm_deg(aVD[2]);
#endif

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
