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

// TODO: Add ECS system-dependency : rigidbody-system.c

static void
fixed_update(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_RIGIDBODY))) return;

    struct ComponentRigidbody *rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    // Calculate simulation-time
    const gsk_Time time = gsk_device_getTime();
    const f64 delta     = time.fixed_delta_time * time.time_scale;

    // --
    // -- Add gravity to net force (mass considered)

    // mass * gravity
    vec3 mG = GLM_VEC3_ZERO_INIT;
    glm_vec3_scale(rigidbody->gravity, rigidbody->mass, mG);
    glm_vec3_add(rigidbody->force, mG, rigidbody->force);

    // --
    // -- Add net force to velocity (mass considered)

    // velocity += force / mass * delta_time;
    vec3 fDm = GLM_VEC3_ZERO_INIT;
    glm_vec3_divs(rigidbody->force, rigidbody->mass, fDm);
    glm_vec3_scale(fDm, delta, fDm);
    glm_vec3_add(rigidbody->velocity, fDm, rigidbody->velocity);
}

void
s_rigidbody_forces_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                            }));
}
