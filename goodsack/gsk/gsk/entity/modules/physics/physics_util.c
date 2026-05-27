/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_util.h"

#define _KINEMATIC_VALS 0

static gsk_DynamicBody
_create_dynamic_body(gsk_Entity entity)
{
    gsk_DynamicBody ret = {0};

    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return ret; }
    gsk_C_Transform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    glm_vec3_copy(cmp_transform->position, ret.position);

    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return ret; }
    gsk_C_Rigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);

#if _KINEMATIC_VALS
    ret.mass         = cmp_rigidbody->mass;
    ret.inverse_mass = cmp_rigidbody->inverse_mass;

    ret.inertia         = cmp_rigidbody->inertia;
    ret.inverse_inertia = cmp_rigidbody->inverse_inertia;

    ret.dynamic_friction = cmp_rigidbody->dynamic_friction;
    ret.static_friction  = cmp_rigidbody->static_friction;
#endif

    if (cmp_rigidbody->is_kinematic == TRUE) { return ret; }

    glm_vec3_copy(cmp_rigidbody->linear_velocity, ret.linear_velocity);
    glm_vec3_copy(cmp_rigidbody->angular_velocity, ret.angular_velocity);

#if !(_KINEMATIC_VALS)
    ret.mass         = cmp_rigidbody->mass;
    ret.inverse_mass = cmp_rigidbody->inverse_mass;

    ret.inertia         = cmp_rigidbody->inertia;
    ret.inverse_inertia = cmp_rigidbody->inverse_inertia;

    ret.dynamic_friction = cmp_rigidbody->dynamic_friction;
    ret.static_friction  = cmp_rigidbody->static_friction;
#endif

    ret.dynamic_friction = cmp_rigidbody->dynamic_friction;
    ret.static_friction  = cmp_rigidbody->static_friction;

    // TODO: probably store reference to entity ID here? Just to avoid possible
    // issues when applying impulses to incorrect bodies

    return ret;
}

gsk_PhysicsMark
gsk_physics_util_create_physics_mark(gsk_Entity entity_a, gsk_Entity entity_b)
{
    gsk_PhysicsMark ret = {0};

    ret.body_a = _create_dynamic_body(entity_a);
    ret.body_b = _create_dynamic_body(entity_b);

    // TODO: calculate relative velocity
    glm_vec3_zero(ret.relative_velocity);

    return ret;
}