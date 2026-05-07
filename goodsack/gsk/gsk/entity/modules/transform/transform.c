/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "transform.h"

#include "core/graphics/mesh/animation.h"
#include "core/graphics/mesh/model.h"
#include "entity/ecs.h"

#include "util/logger.h"

// #define ECS_SYSTEM
//  ECS_SYSTEM_DECLARE()

static void
_validate_bone_matrix(gsk_Entity e)
{
    if (!gsk_ecs_has(e, C_BONE_ATTACHMENT)) { return; }
    gsk_C_BoneAttachment *c_bone_attachment = gsk_ecs_get(e, C_BONE_ATTACHMENT);

    c_bone_attachment->p_joint = NULL;

    gsk_Entity ent_skeleton =
      gsk_ecs_ent(e.ecs, c_bone_attachment->entity_skeleton);

    // Get joint from skeleton
    if (!gsk_ecs_has(ent_skeleton, C_MODEL))
    {
        LOG_ERROR("ent_skeleton does not have a Mesh component!");
        return;
    }
    struct ComponentModel *cmp_model = gsk_ecs_get(ent_skeleton, C_MODEL);
    gsk_Model *pmdl                  = cmp_model->pModel;
    gsk_Mesh *pmsh                   = pmdl->meshes[0];

    // TODO: should check skeleton on component instead
    if (!pmsh->meshData->isSkinnedMesh)
    {
        LOG_ERROR("Attempting to attach to non skinned-mesh!");
        return;
    }

    gsk_Joint *joint = NULL;
    if (cmp_model->_skeleton)
    {
        joint = ((gsk_Skeleton *)(cmp_model->_skeleton))
                  ->joints[c_bone_attachment->bone_id];
    }

    if (joint == NULL)
    {
        LOG_ERROR("Failed to get joint by bone_id: %d",
                  c_bone_attachment->bone_id);
        return;
    }

    c_bone_attachment->p_joint = joint;
}

static void
_set_to_joint_matrix(gsk_Entity e, mat4 *m4i, mat4 *skinned)
{
    if (!gsk_ecs_has(e, C_BONE_ATTACHMENT)) { return; }
    gsk_C_BoneAttachment *c_bone_attachment = gsk_ecs_get(e, C_BONE_ATTACHMENT);

    if (c_bone_attachment->p_joint == NULL) { return; }

    gsk_Entity ent_skeleton =
      gsk_ecs_ent(e.ecs, c_bone_attachment->entity_skeleton);

    gsk_C_Transform *skel_transform = gsk_ecs_get(ent_skeleton, C_TRANSFORM);

    gsk_Joint *joint = c_bone_attachment->p_joint;

    glm_mat4_copy(joint->pose.mTransform, *skinned);
    glm_mat4_copy(skel_transform->model, *m4i);
}

static void
__update_world_position(struct ComponentTransform *cmp_transform)
{
    cmp_transform->world_position[0] = cmp_transform->model[3][0];
    cmp_transform->world_position[1] = cmp_transform->model[3][1];
    cmp_transform->world_position[2] = cmp_transform->model[3][2];
}

// TODO: should be a utility function elsewhere, probably
static inline f32
__fdiv_safe(const f32 a, const f32 b)
{
    return (b == 0) ? 0 : a / b;
}

static void
__update_rotation_euler_angles(struct ComponentTransform *cmp_transform)
{
    // glm_quat_normalize(cmp_transform->rotation);

    glm_quat_mat4(cmp_transform->rotation, cmp_transform->m4_rotation);

    // update euler orientation
    vec3 new_angles = {0, 0, 0};
    glm_euler_angles(cmp_transform->m4_rotation, new_angles);
    cmp_transform->orientation[0] = glm_deg(new_angles[0]);
    cmp_transform->orientation[1] = glm_deg(new_angles[1]);
    cmp_transform->orientation[2] = glm_deg(new_angles[2]);
}

static void
__quat_from_xyz(vec3 euler, versor *dest)
{
    versor delta = GLM_QUAT_IDENTITY_INIT;
    {
        versor qx = GLM_QUAT_IDENTITY_INIT;
        versor qy = GLM_QUAT_IDENTITY_INIT;
        versor qz = GLM_QUAT_IDENTITY_INIT;

        // 1. Create individual rotations (angles must be in RADIANS)
        glm_quatv(qx, glm_rad(euler[0]), (vec3) {1.0f, 0.0f, 0.0f});
        glm_quatv(qy, glm_rad(euler[1]), (vec3) {0.0f, 1.0f, 0.0f});
        glm_quatv(qz, glm_rad(euler[2]), (vec3) {0.0f, 0.0f, 1.0f});

        versor temp = GLM_QUAT_IDENTITY_INIT;
        // glm_quat_mul(qz, qy, temp);
        // glm_quat_mul(temp, qx, delta);
        glm_quat_mul(qx, qy, temp);
        glm_quat_mul(temp, qz, dest);
        glm_quat_normalize(dest);
    }
}

static void
__rotate_euler(vec3 euler, versor *p_cnt_rotation)
{
    // glm_quat_copy(delta, *p_cnt_rotation);
    // return;

    versor next_rot = GLM_QUAT_IDENTITY_INIT;
    __quat_from_xyz(euler, next_rot);

    glm_quat_mul(next_rot, *p_cnt_rotation, next_rot);
    glm_quat_normalize(next_rot);

    glm_quat_copy(next_rot, *p_cnt_rotation);
}

void
transform_set_rotation(struct ComponentTransform *transform, versor quat)
{
    versor new_rot = GLM_QUAT_IDENTITY_INIT;

    glm_quat_normalize_to(quat, new_rot);
    glm_quat_copy(new_rot, transform->rotation);

    __update_rotation_euler_angles(transform);
}

void
transform_set_rotation_xyz(struct ComponentTransform *transform, vec3 rotation)
{
    versor new_rot = GLM_QUAT_IDENTITY_INIT;
    __quat_from_xyz(rotation, new_rot);
    transform_set_rotation(transform, new_rot);
}

void
transform_rotate(struct ComponentTransform *transform, vec3 rotation)
{
    __rotate_euler(rotation, &transform->rotation);
    __update_rotation_euler_angles(transform);
}

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
    transform->has_parent =
      (transform->parent_entity_id >= ECS_ID_FIRST) ? TRUE : FALSE;

    mat4 m4i = GLM_MAT4_IDENTITY_INIT;

    versor rot_init = GLM_QUAT_IDENTITY_INIT;
    glm_quat_copy(rot_init, transform->rotation);
    transform_rotate(transform, transform->orientation);

    // Get parent transform (if exists)
    if (transform->has_parent)
    {
        gsk_Entity ent_parent = gsk_ecs_ent(e.ecs, transform->parent_entity_id);

        if (!gsk_ecs_has(ent_parent, C_TRANSFORM))
        {
            transform->has_parent       = FALSE;
            transform->parent_entity_id = 0;
        }

        struct ComponentTransform *parentTransform =
          gsk_ecs_get(ent_parent, C_TRANSFORM);
        glm_mat4_copy(parentTransform->model, m4i);
    }

    glm_translate(m4i, transform->position);
    glm_mat4_copy(m4i, transform->model);
    __update_world_position(transform);

    // stupid hack which basically doesn't allow a zero scale.
    if (!transform->scale[0] && !transform->scale[1] && !transform->scale[2])
    {
        glm_vec3_one(transform->scale);
    }

    // TODO: [vulkan] Make descriptor set HERE

    // set the default forward vector
    glm_vec3_zero(transform->forward);

    // Validate BONE_ATTACHMENT (ensure bone exists)
    _validate_bone_matrix(e);
}

static void
late_update(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
    transform->has_parent =
      (transform->parent_entity_id >= ECS_ID_FIRST) ? TRUE : FALSE;

    mat4 m4i     = GLM_MAT4_IDENTITY_INIT;
    mat4 skinned = GLM_MAT4_IDENTITY_INIT;

    // check for BONE_ATTACHMENT
    _set_to_joint_matrix(e, &m4i, &skinned);

    vec3 scale_scalar = {0, 0, 0};

    if (gsk_ecs_has(e, C_CAMERA))
    {
        struct ComponentCamera *camera = gsk_ecs_get(e, C_CAMERA);
        glm_mat4_inv(camera->view, transform->model);
        __update_world_position(transform);
        return;
    }

    if (transform->has_parent)
    {
        gsk_Entity ent_parent = gsk_ecs_ent(e.ecs, transform->parent_entity_id);

        if (!gsk_ecs_has(ent_parent, C_TRANSFORM))
        {
            gsk_ecs_ent_destroy(e);
            return;
        }

        struct ComponentTransform *parentTransform =
          gsk_ecs_get(ent_parent, C_TRANSFORM);

        glm_mat4_copy(parentTransform->model, m4i);

        scale_scalar[0] =
          __fdiv_safe(transform->scale[0], parentTransform->scale[0]);
        scale_scalar[1] =
          __fdiv_safe(transform->scale[1], parentTransform->scale[1]);
        scale_scalar[2] =
          __fdiv_safe(transform->scale[2], parentTransform->scale[2]);

    } else
    {
        glm_vec3_copy(transform->scale, scale_scalar);
    }

    glm_mat4_mul(m4i, skinned, m4i);
    glm_translate(m4i, transform->position);

    mat4 mat_rot = GLM_MAT4_IDENTITY_INIT;

#if _TRANSFORM_QUATERNION
    __update_rotation_euler_angles(transform);
    glm_mat4_copy(transform->m4_rotation, mat_rot);

#else
    glm_rotate_x(mat_rot, glm_rad(transform->orientation[0]), mat_rot);
    glm_rotate_y(mat_rot, glm_rad(transform->orientation[1]), mat_rot);
    glm_rotate_z(mat_rot, glm_rad(transform->orientation[2]), mat_rot);

    glm_mat4_copy(mat_rot, transform->m4_rotation);
#endif // _TRANSFORM_QUATERNION

    // separated rotation matrix
    glm_mat4_mul(m4i, mat_rot, m4i);

    glm_scale(m4i, scale_scalar);

    glm_mat4_copy(m4i, transform->model);

    // set world position
    __update_world_position(transform);

    // get the forward vector from the rotation matrix
    transform->forward[0] = mat_rot[2][2];
    transform->forward[1] = -mat_rot[2][1];
    transform->forward[2] = -mat_rot[2][0];
}

void
s_transform_init(gsk_ECS *ecs)
{
    //_ECS_DECL_COMPONENT(ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init        = (gsk_ECSSubscriber)init,
                              .destroy     = NULL,
                              .render      = NULL,
                              .update      = NULL,
                              .late_update = (gsk_ECSSubscriber)late_update,
                            }));
}
