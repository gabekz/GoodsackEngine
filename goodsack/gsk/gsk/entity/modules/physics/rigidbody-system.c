/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Move solvers to separate file

#include "rigidbody-system.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "tools/debug/debug_context.h"
#include "tools/debug/debug_draw_line.h"

#include "core/device/device.h"
#include "physics/physics_solver.h"

// Functionality toggles
#define USING_FRICTION 1
#define DEBUG_POINTS   3 // 0 -- OFF | value = entity id

// physics default values
#define DEFAULT_RESTITUION       0.2f
#define DEFAULT_STATIC_FRICTION  1.0f
#define DEFAULT_DYNAMIC_FRICTION 1.0f

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
_impulse_solver_with_rotation(_SolverData solver_data);

static void
_impulse_solver_with_rotation_friction(_SolverData solver_data);

//-----------------------------------------------------------------------------
static void
__debug_points(const _SolverData solver_data, u32 id)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_Entity entity                     = solver_data.entity;

#if 1
    if (entity.id == id) {
        // relative velocity
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_RAY,
                               entity.id + 1,
                               solver_data.p_transform->position,
                               collision_result->physics_mark.relative_velocity,
                               1,
                               (vec4) {0, 1, 1, 1},
                               FALSE);

        // collision normal
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_RAY,
                               entity.id + 2,
                               solver_data.p_transform->position,
                               collision_result->points.normal,
                               5,
                               (vec4) {0, 1, 0, 1},
                               FALSE);

#if 0
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               entity.id + 2,
                               collision_result->points.point_b,
                               (vec4) {1, 0, 0, 0},
                               FALSE);
#endif
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
    rigidbody->static_friction  = DEFAULT_STATIC_FRICTION;
    rigidbody->dynamic_friction = DEFAULT_DYNAMIC_FRICTION;

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
        _impulse_solver_with_rotation(solver_data);

#if USING_FRICTION
        _impulse_solver_with_rotation_friction(solver_data);
#endif // USING_FRICTION

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
static void
_impulse_solver_with_rotation(_SolverData solver_data)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker                = collision_result->physics_mark;

    struct ComponentTransform *transform   = solver_data.p_transform;
    struct ComponentRigidbody *rigidbody_a = solver_data.p_rigidbody;
    const f64 delta                        = solver_data.delta;

    gsk_DynamicBody body_a = marker.body_a;
    gsk_DynamicBody body_b = marker.body_b;

    // --
    // Get point of contact and perps - calculate relative velocity

    vec3 ra, ra_perp, rb, rb_perp;
    // RA
    // glm_vec3_subs(
    //  collision_result->points.point_a, collision_result->points.depth, ra);
    glm_vec3_sub(collision_result->points.point_a, body_a.position, ra);

    // perpendicular
    glm_vec3_cross(ra, body_a.position, ra_perp);

    // RB
    // glm_vec3_subs(
    //  collision_result->points.point_b, collision_result->points.depth, rb);
    glm_vec3_sub(collision_result->points.point_b, body_b.position, rb);

    // perpendicular
    glm_vec3_cross(rb, body_b.position, rb_perp);

#if 0
    if (solver_data.entity.id == DEBUG_POINTS) {
        gsk_debug_markers_push(solver_data.entity.ecs->renderer->debugContext,
                               solver_data.entity.id + 3,
                               ra_perp,
                               (vec4) {0, 1, 1, 1},
                               FALSE);
    }
#endif

    // angular velocity
    vec3 angular_linear_velocity_a, angular_linear_velocity_b;
    glm_vec3_mul(ra_perp, body_a.angular_velocity, angular_linear_velocity_a);
    glm_vec3_mul(rb_perp, body_b.angular_velocity, angular_linear_velocity_b);

    // calculate relative velocity
    /* (a_vel + a_ang_vel) - (b_vel + b_ang_vel) - */

    vec3 relative_velocity;
    vec3 cmba, cmbb;
    glm_vec3_add(body_a.linear_velocity, angular_linear_velocity_a, cmba);
    glm_vec3_add(body_b.linear_velocity, angular_linear_velocity_b, cmbb);
    glm_vec3_sub(cmba, cmbb, relative_velocity);

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(collision_result->points.normal, collision_normal);

    float vDotN = (glm_vec3_dot(relative_velocity, collision_normal));

    // dots
    f32 ra_perpDotN = glm_dot(ra_perp, collision_normal);
    f32 rb_perpDotN = glm_dot(rb_perp, collision_normal);

    f32 denom = body_a.inverse_mass + body_b.inverse_mass +
                (pow(ra_perpDotN, 2) * body_a.inverse_inertia) +
                (pow(rb_perpDotN, 2) * body_b.inverse_inertia);

    // --
    // Basic collision reflection

    float restitution = DEFAULT_RESTITUION; // Bounce factor

    // calculate relative velocity
    float F = -(1.0f + restitution) * vDotN;
    F /= denom;
    // F /= contact_points_total; -- possibly needed
    // prevent negative impulse
    if (vDotN >= 0.0f) { F = 0; } // F = 0; }

    vec3 impulse = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(collision_normal, F, impulse);

    vec3 torque;
    glm_vec3_cross(ra, impulse, torque);
    glm_vec3_negate(torque);
    // scale torque by inverse inertia
    glm_vec3_scale(torque, body_a.inverse_inertia, torque);

    // scale impulse by inverse mass
    glm_vec3_scale(impulse, body_a.inverse_mass, impulse);

    // --
    // Apply impulses

    glm_vec3_add(
      rigidbody_a->linear_velocity, impulse, rigidbody_a->linear_velocity);

    glm_vec3_add(
      rigidbody_a->angular_velocity, torque, rigidbody_a->angular_velocity);
}
//-----------------------------------------------------------------------------
static void
_impulse_solver_with_rotation_friction(_SolverData solver_data)
{
    gsk_CollisionResult *collision_result = solver_data.p_collision_result;
    gsk_PhysicsMark marker                = collision_result->physics_mark;

    struct ComponentTransform *transform   = solver_data.p_transform;
    struct ComponentRigidbody *rigidbody_a = solver_data.p_rigidbody;
    const f64 delta                        = solver_data.delta;

    gsk_DynamicBody body_a = marker.body_a;
    gsk_DynamicBody body_b = marker.body_b;

    // --
    // Get point of contact and perps - calculate relative velocity

    vec3 ra, ra_perp, rb, rb_perp;
    // RA
    // glm_vec3_subs(
    //  collision_result->points.point_a, collision_result->points.depth, ra);
    glm_vec3_sub(collision_result->points.point_a, body_a.position, ra);

    // perpendicular
    glm_vec3_cross(ra, body_a.position, ra_perp);

    // RB
    // glm_vec3_subs(
    //  collision_result->points.point_b, collision_result->points.depth, rb);
    glm_vec3_sub(collision_result->points.point_b, body_b.position, rb);

    // perpendicular
    glm_vec3_cross(rb, body_b.position, rb_perp);

#if 0
    if (solver_data.entity.id == DEBUG_POINTS) {
        gsk_debug_markers_push(solver_data.entity.ecs->renderer->debugContext,
                               solver_data.entity.id + 3,
                               ra_perp,
                               (vec4) {0, 1, 1, 1},
                               FALSE);
    }
#endif

    // angular velocity
    vec3 angular_linear_velocity_a, angular_linear_velocity_b;
    glm_vec3_mul(ra_perp, body_a.angular_velocity, angular_linear_velocity_a);
    glm_vec3_mul(rb_perp, body_b.angular_velocity, angular_linear_velocity_b);

    // calculate relative velocity
    /* (a_vel + a_ang_vel) - (b_vel + b_ang_vel) - */

    vec3 relative_velocity;
    vec3 cmba, cmbb;
    glm_vec3_add(body_a.linear_velocity, angular_linear_velocity_a, cmba);
    glm_vec3_add(body_b.linear_velocity, angular_linear_velocity_b, cmbb);
    glm_vec3_sub(cmba, cmbb, relative_velocity);

    vec3 collision_normal = GLM_VEC3_ZERO_INIT;
    glm_vec3_copy(collision_result->points.normal, collision_normal);

    // --
    // calculate tangent

    vec3 tangent = GLM_VEC3_ZERO_INIT;

    // tangent = velocity - dot(velocity, normal) * normal;
    glm_vec3_scale(collision_normal,
                   glm_vec3_dot(relative_velocity, collision_normal),
                   tangent);
    glm_vec3_sub(relative_velocity, tangent, tangent);

    // check tangent for near-zero
    float zerodist = glm_vec3_distance(tangent, GLM_VEC3_ZERO);
    // assert(zerodist >= 0.0f);
    // LOG_INFO("%f zerodist", zerodist);
    if (zerodist <= 0.002f) { return; } // get out!

    glm_vec3_normalize(tangent);

    float vDotN = (glm_vec3_dot(relative_velocity, collision_normal));
    float vDotT = (glm_vec3_dot(relative_velocity, tangent));

    // prevent negative impulse
    float restitution = DEFAULT_RESTITUION; // Bounce factor
    float F           = -(1.0f + restitution) * vDotN;
    // if (vDotN >= 0.0f) { return; };
    // if (vDotN >= 0) F = 0;

    // dots
    f32 ra_perpDotT = glm_dot(ra_perp, tangent);
    f32 rb_perpDotT = glm_dot(rb_perp, tangent);

    f32 denom = body_a.inverse_mass + body_b.inverse_mass +
                (pow(ra_perpDotT, 2) * body_a.inverse_inertia) +
                (pow(rb_perpDotT, 2) * body_b.inverse_inertia);

    // --
    // Basic collision reflection

    // calculate relative velocity
    float Ft = -vDotT;
    F /= denom;
    Ft /= denom;
    // F /= contact_points_total; -- possibly needed

    f32 sf =
      (rigidbody_a->static_friction + rigidbody_a->static_friction) * 0.5f;
    f32 df =
      (rigidbody_a->dynamic_friction + rigidbody_a->dynamic_friction) * 0.5f;

    // calculate friction_impulse
    vec3 friction_impulse = GLM_VEC3_ZERO_INIT;

    if (abs(Ft) <= F * sf) {
        glm_vec3_scale(tangent, Ft, friction_impulse);
        // LOG_INFO("STATIC");
    } else {
        glm_vec3_scale(tangent, F * df, friction_impulse);
        // LOG_INFO("DYNAMIC");
    }

    vec3 torque;
    glm_vec3_cross(ra, friction_impulse, torque);
    glm_vec3_negate(torque);
    // scale torque by inverse inertia
    glm_vec3_scale(torque, body_a.inverse_inertia, torque);

    // scale impulse by inverse mass
    glm_vec3_scale(friction_impulse, body_a.inverse_mass, friction_impulse);

    // --
    // Apply impulses

    glm_vec3_sub(rigidbody_a->linear_velocity,
                 friction_impulse,
                 rigidbody_a->linear_velocity);

    glm_vec3_add(
      rigidbody_a->angular_velocity, torque, rigidbody_a->angular_velocity);
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
