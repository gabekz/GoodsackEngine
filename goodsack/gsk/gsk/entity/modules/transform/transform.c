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

void
transform_translate(struct ComponentTransform *transform, vec3 position)
{
    glm_translate(transform->model, position);
}

void
transform_position(struct ComponentTransform *transform, vec3 position)
{
    mat4 matrix = GLM_MAT4_IDENTITY_INIT;
    glm_translate(matrix, position);
    glm_vec3_copy(position, transform->position);
    glm_mat4_copy(matrix, transform->model);
}

/*
void transform_rotate(struct ComponentTransform *transform, vec3 rotation) {
    glm_rotate(*transform->mvp.matrix, rotation);
}
*/

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
    gsk_Model *pmdl =
      ((struct ComponentModel *)gsk_ecs_get(ent_skeleton, C_MODEL))->pModel;
    gsk_Mesh *pmsh = pmdl->meshes[0];

    if (!pmsh->meshData->isSkinnedMesh)
    {
        LOG_ERROR("Attempting to attach to non skinned-mesh!");
        return;
    }

    gsk_Joint *joint =
      pmsh->meshData->skeleton->joints[c_bone_attachment->bone_id];

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

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
    transform->has_parent                = false; // if not already set..

    mat4 m4i = GLM_MAT4_IDENTITY_INIT;

    // Get parent transform (if exists)
    if (transform->parent_entity_id >= ECS_ID_FIRST)
    {
        transform->has_parent                      = true;
        struct ComponentTransform *parentTransform = gsk_ecs_get(
          gsk_ecs_ent(e.ecs, transform->parent_entity_id), C_TRANSFORM);
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

    mat4 m4i     = GLM_MAT4_IDENTITY_INIT;
    mat4 skinned = GLM_MAT4_IDENTITY_INIT;

    // check for BONE_ATTACHMENT
    _set_to_joint_matrix(e, &m4i, &skinned);

    if (gsk_ecs_has(e, C_CAMERA))
    {
        struct ComponentCamera *camera = gsk_ecs_get(e, C_CAMERA);
        glm_mat4_inv(camera->view, transform->model);
        __update_world_position(transform);
        return;
    }

    if (transform->has_parent)
    {
        struct ComponentTransform *parentTransform = gsk_ecs_get(
          gsk_ecs_ent(e.ecs, transform->parent_entity_id), C_TRANSFORM);
        glm_mat4_copy(parentTransform->model, m4i);
    }

    glm_mat4_mul(m4i, skinned, m4i);
    glm_translate(m4i, transform->position);

    mat4 mat_rot = GLM_MAT4_IDENTITY_INIT;
    glm_rotate_x(mat_rot, glm_rad(transform->orientation[0]), mat_rot);
    glm_rotate_y(mat_rot, glm_rad(transform->orientation[1]), mat_rot);
    glm_rotate_z(mat_rot, glm_rad(transform->orientation[2]), mat_rot);

    // separated rotation matrix
    glm_mat4_mul(m4i, mat_rot, m4i);

    glm_scale(m4i, transform->scale);

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
