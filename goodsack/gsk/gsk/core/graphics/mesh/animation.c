/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: get rid of code duplication between keyframe and keyframe_lerp

#include "animation.h"

#include <string.h>

#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/mesh_helpers.inl"

void
gsk_animation_set_keyframe(gsk_Skeleton *p_skeleton,
                           gsk_Animation *animation,
                           u32 keyframe)
{
    // TODO: remove
    gsk_Skeleton *skeleton = p_skeleton;

    for (int i = 0; i < skeleton->jointsCount; i++)
    {

        gsk_Pose cnt_pose = skeleton->joints[i]->pose;
        gsk_Pose nxt_pose = *animation->keyframes[keyframe]->poses[i];

        // TODO: CLEAN THIS UP ASAP
        if (skeleton->joints[i]->override == TRUE) { nxt_pose = cnt_pose; }

        gsk_Pose new_pose         = nxt_pose;
        skeleton->joints[i]->pose = nxt_pose;

        mat4 matrix = GLM_MAT4_ZERO_INIT;

        // World
        _joint_transform_world(skeleton->joints[i], (float *)matrix);

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
gsk_animation_set_keyframe_lerp(gsk_Skeleton *p_skeleton,
                                gsk_Animation *animation,
                                u32 k0,     // previous keyframe index
                                u32 k1,     // next keyframe index
                                float ratio // in [0, 1]
)
{

    // TODO: remove
    gsk_Skeleton *skeleton = p_skeleton;

    for (int i = 0; i < skeleton->jointsCount; i++)
    {
        gsk_Pose poseA = *animation->keyframes[k0]->poses[i]; // local
        gsk_Pose poseB = *animation->keyframes[k1]->poses[i]; // local

#if 1
        // TODO: CLEAN THIS UP ASAP
        if (skeleton->joints[i]->override == TRUE)
        {
            gsk_animation_set_keyframe(p_skeleton, animation, k0);
            continue;
        }
#endif

        gsk_Pose newPose;

        // Interpolate translation & scale linearly
        glm_vec3_lerp(
          poseA.translation, poseB.translation, ratio, newPose.translation);
        glm_vec3_lerp(poseA.scale, poseB.scale, ratio, newPose.scale);

        // But do a slerp for rotation
        glm_quat_slerp(poseA.rotation, poseB.rotation, ratio, newPose.rotation);
        glm_quat_normalize(newPose.rotation);

        newPose.hasMatrix         = 0;
        skeleton->joints[i]->pose = newPose;
    }

    // Then build final *global* transforms
    for (int i = 0; i < skeleton->jointsCount; i++)
    {
        mat4 transformMatrix = GLM_MAT4_ZERO_INIT;
        _joint_transform_world(skeleton->joints[i], (float *)transformMatrix);

        glm_mat4_copy(transformMatrix, skeleton->joints[i]->pose.mTransform);

        // And compute skinning
        glm_mat4_mul(skeleton->joints[i]->pose.mTransform,
                     skeleton->joints[i]->mInvBindPose,
                     skeleton->joints[i]->pose.mSkinningMatrix);
    }
}