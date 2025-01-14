/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: get rid of code duplication between keyframe and keyframe_lerp

#include "animation.h"

#include <string.h>

#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/mesh_helpers.inl"

// TODO: Fix lerp
#define DISABLE_LERP 1

void
gsk_animation_set_keyframe(gsk_Animation *animation, u32 keyframe)
{
    gsk_Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++)
    {

        gsk_Pose cnt_pose = skeleton->joints[i]->pose;
        gsk_Pose nxt_pose = *animation->keyframes[keyframe]->poses[i];

        gsk_Pose new_pose         = nxt_pose;
        skeleton->joints[i]->pose = nxt_pose;

        mat4 matrix = GLM_MAT4_ZERO_INIT;
#if 0
        // Local
        _joint_transform_local(skeleton->joints[i],
                               (float *)Matrix);
#else
        // World
        _joint_transform_world(skeleton->joints[i], (float *)matrix);
#endif

        // set as current pose
        glm_mat4_copy(matrix, skeleton->joints[i]->pose.mTransform);
    }

    // calculate skinning matrix
    for (int i = 0; i < skeleton->jointsCount; i++)
    {
        glm_mat4_mul(skeleton->joints[i]->pose.mTransform,
                     skeleton->joints[i]->mInvBindPose,
                     skeleton->joints[i]->pose.mSkinningMatrix);
    }
}

void
gsk_animation_set_keyframe_lerp(gsk_Animation *animation,
                                u32 keyframe,
                                float ratio)
{

#if DISABLE_LERP
    gsk_animation_set_keyframe(animation, keyframe - 1);
    return;
#endif

    gsk_Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++)
    {

        gsk_Pose currentPose = skeleton->joints[i]->pose;
        gsk_Pose nextPose    = *animation->keyframes[keyframe]->poses[i];

        gsk_Pose newPose;

        glm_vec3_lerp(currentPose.translation,
                      nextPose.translation,
                      ratio,
                      newPose.translation);
        glm_vec4_lerp(
          currentPose.rotation, nextPose.rotation, ratio, newPose.rotation);
        glm_vec3_lerp(currentPose.scale, nextPose.scale, ratio, newPose.scale);
        newPose.hasMatrix         = 0;
        skeleton->joints[i]->pose = newPose; // must be set for parent
                                             // transform-correction. Maybe slow

        mat4 transformMatrix = GLM_MAT4_ZERO_INIT;
        _joint_transform_world(skeleton->joints[i], (float *)transformMatrix);

#if 0
        // Set to model matrix
            mat4 init = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_mul(skeleton->rootMatrix, transformMatrix, transformMatrix);
#endif

        // set as current pose
        glm_mat4_copy(transformMatrix, skeleton->joints[i]->pose.mTransform);
    }

    // calculate skinning matrix
    for (int i = 0; i < skeleton->jointsCount; i++)
    {
        glm_mat4_mul(skeleton->joints[i]->pose.mTransform,
                     skeleton->joints[i]->mInvBindPose,
                     skeleton->joints[i]->pose.mSkinningMatrix);
    }
}

void
gsk_skeleton_set_animation(gsk_Skeleton *p_skeleton, u32 index)
{
    if (index > p_skeleton->animations_count - 1)
    {
        LOG_ERROR(
          "Failed to set animation on skeleton %s. Index %d is out of range.",
          p_skeleton->name,
          index);
        return;
    }

    p_skeleton->animation           = p_skeleton->p_animations[index];
    p_skeleton->cnt_animation_index = index;
}
