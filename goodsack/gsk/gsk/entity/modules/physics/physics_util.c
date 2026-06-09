/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "physics_util.h"

#include "entity/modules/transform/transform.h"
#include "physics/physics_sat.h"

#include "runtime/gsk_runtime_debug.h"

#include "util/vec_colors.h"

#define _KINEMATIC_VALS 0

static void
_box_bounds_center_world(const gsk_BoxCollider *col,
                         vec3 pos,
                         mat3 rot,
                         vec3 out)
{
    vec3 local_center;
    glm_vec3_add(col->bounds[0], col->bounds[1], local_center);
    glm_vec3_scale(local_center, 0.5f, local_center);

    vec3 offset = GLM_VEC3_ZERO_INIT;
    vec3 term;

    glm_vec3_scale(rot[0], local_center[0], offset);

    glm_vec3_scale(rot[1], local_center[1], term);
    glm_vec3_add(offset, term, offset);

    glm_vec3_scale(rot[2], local_center[2], term);
    glm_vec3_add(offset, term, offset);

    glm_vec3_add(pos, offset, out);
}

gsk_DynamicBody
gsk_physics_util_create_dynamic_body(gsk_Entity entity)
{
    gsk_DynamicBody ret = {0};

    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return ret; }
    gsk_C_Transform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    // TODO: ret.position should actually be center-of-mass (with respect to
    // bounds if BOX Collider)
    glm_vec3_copy(cmp_transform->position, ret.position);
    glm_vec3_copy(ret.position, ret.center_of_mass);

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

    mat3 rot = GLM_MAT3_IDENTITY_INIT;
    glm_quat_mat3(cmp_transform->rotation, rot);

    gsk_physics_util_inverse_inertia_world(
      rot, cmp_rigidbody->inverse_inertia_tensor, ret.inertia_tensor);

#if 1

    if (gsk_ecs_has(entity, C_COLLIDER))
    {
        gsk_C_Collider *cmp_collider = gsk_ecs_get(entity, C_COLLIDER);

        if (cmp_collider->type == COLLIDER_BOX)
        {
            gsk_OBB obb = gsk_physics_sat_obb_make(
              ((gsk_Collider *)cmp_collider->pCollider)->collider_data,
              ret.position,
              rot);

            // glm_vec3_add(ret.position, obb.e, ret.position);

            vec3 out = {0, 0, 0};
            _box_bounds_center_world(
              ((gsk_Collider *)cmp_collider->pCollider)->collider_data,
              ret.position,
              rot,
              out);

            // GSK_DEBUG_DRAW_POINT(out, 1.0f, VCOL_ORANGE);

            glm_vec3_copy(obb.c, ret.center_of_mass);
            // glm_vec3_copy(out, ret.position);
            // GSK_DEBUG_DRAW_POINT(ret.position, 1.0f, VCOL_CYAN);
        }
    }
#endif

    if (entity.id > 305)
    {

        GSK_DEBUG_DRAW_POINT(ret.position, 1.0f, VCOL_CYAN);
        GSK_DEBUG_DRAW_POINT(ret.center_of_mass, 1.0f, VCOL_ORANGE);
    }

    return ret;
}

gsk_PhysicsMark
gsk_physics_util_create_physics_mark(gsk_Entity entity_a, gsk_Entity entity_b)
{
    gsk_PhysicsMark ret = {0};

    ret.body_a = gsk_physics_util_create_dynamic_body(entity_a);
    ret.body_b = gsk_physics_util_create_dynamic_body(entity_b);

    // TODO: calculate relative velocity
    glm_vec3_zero(ret.relative_velocity);

    return ret;
}

void
gsk_physics_util_inverse_inertia_world(mat3 rot,
                                       mat3 inv_inertia_local,
                                       mat3 out_inv_inertia_world)
{
    mat3 rot_t;
    mat3 tmp;

    glm_mat3_transpose_to(rot, rot_t);

    // out = R * I_local_inv * R^T
    glm_mat3_mul(rot, inv_inertia_local, tmp);
    glm_mat3_mul(tmp, rot_t, out_inv_inertia_world);
}

void
gsk_physics_util_relative_velocity(gsk_DynamicBody body_a,
                                   gsk_DynamicBody body_b,
                                   vec3 point_a,
                                   vec3 point_b,
                                   vec3 out_relative_velocity,
                                   vec3 out_ra,
                                   vec3 out_rb)
{
    vec3 ra, ra_perp;
    vec3 rb, rb_perp;

    // calculate r-values + relative velocity
    {
        glm_vec3_sub(point_a, body_a.center_of_mass, ra);
        glm_vec3_cross(body_a.angular_velocity, ra, ra_perp);

        glm_vec3_sub(point_b, body_b.center_of_mass, rb);
        glm_vec3_cross(body_b.angular_velocity, rb, rb_perp);

        vec3 cmba, cmbb;
        glm_vec3_add(body_a.linear_velocity, ra_perp, cmba);
        glm_vec3_add(body_b.linear_velocity, rb_perp, cmbb);
        glm_vec3_sub(cmba, cmbb, out_relative_velocity);

        glm_vec3_copy(ra, out_ra);
        glm_vec3_copy(rb, out_rb);
    }
}

f32
gsk_physics_util_effective_mass_axis(
  gsk_DynamicBody *body_a, gsk_DynamicBody *body_b, vec3 ra, vec3 rb, vec3 axis)
{
    float k = body_a->inverse_mass + body_b->inverse_mass;

    // A angular term: dot(n, (I^-1 * (ra x n)) x ra)
    vec3 ra_cross_n;
    vec3 ia_ra_cross_n;
    vec3 angular_a;

    glm_vec3_cross(ra, axis, ra_cross_n);
    glm_mat3_mulv(body_a->inertia_tensor, ra_cross_n, ia_ra_cross_n);
    glm_vec3_cross(ia_ra_cross_n, ra, angular_a);

    k += glm_vec3_dot(axis, angular_a);

    // B angular term: dot(n, (I^-1 * (rb x n)) x rb)
    vec3 rb_cross_n;
    vec3 ib_rb_cross_n;
    vec3 angular_b;

    glm_vec3_cross(rb, axis, rb_cross_n);
    glm_mat3_mulv(body_b->inertia_tensor, rb_cross_n, ib_rb_cross_n);
    glm_vec3_cross(ib_rb_cross_n, rb, angular_b);

    k += glm_vec3_dot(axis, angular_b);

    return k;
}
