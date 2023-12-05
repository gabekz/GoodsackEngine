/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "animation.h"

#include <string.h>

#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/mesh_helpers.inl"

void
gsk_animation_set_keyframe(gsk_Animation *animation, u32 keyframe)
{
    gsk_Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++) {

        gsk_Pose currentPose = skeleton->joints[i]->pose;
        gsk_Pose nextPose    = *animation->keyframes[keyframe]->poses[i];

        // gsk_Pose newPose = LERP(currentPose, newPose, t);
        gsk_Pose newPose          = nextPose;
        skeleton->joints[i]->pose = nextPose;

        // Local
        mat4 transformMatrixLocal = GLM_MAT4_ZERO_INIT;
        _joint_transform_local(skeleton->joints[i],
                               (float *)transformMatrixLocal);

        // calculate skinning matrix
        glm_mat4_mul(transformMatrixLocal,
                     skeleton->joints[i]->mInvBindPose,
                     skeleton->joints[i]->pose.mSkinningMatrix);
    }
}

void
gsk_animation_set_keyframe_lerp(gsk_Animation *animation,
                                u32 keyframe,
                                float ratio)
{
    gsk_Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++) {

        u32 keyframeActual   = keyframe;
        gsk_Pose currentPose = *animation->keyframes[keyframe - 1]->poses[i];
        gsk_Pose nextPose    = *animation->keyframes[keyframe]->poses[i];

        gsk_Pose newPose;

        glm_vec3_lerp(currentPose.translation,
                      nextPose.translation,
                      ratio,
                      newPose.translation);
        glm_vec4_lerp(
          currentPose.rotation, nextPose.rotation, ratio, newPose.rotation);
        glm_vec3_lerp(currentPose.scale, nextPose.scale, ratio, newPose.scale);
        newPose.hasMatrix = 0;
        skeleton->joints[i]->pose =
          newPose; // must be set for parent transform-correction. Maybe slow

        mat4 transformMatrix = GLM_MAT4_ZERO_INIT;
        _joint_transform_world(skeleton->joints[i], (float *)transformMatrix);

#if 0
        // Set to model matrix
            mat4 init = GLM_MAT4_IDENTITY_INIT;
            glm_mat4_mul(skeleton->rootMatrix, transformMatrix, transformMatrix);
#endif

        glm_mat4_copy(
          transformMatrix,
          skeleton->joints[i]->pose.mTransform); // set as current pose
    }

    // calculate skinning matrix
    for (int i = 0; i < skeleton->jointsCount; i++) {
        glm_mat4_mul(skeleton->joints[i]->pose.mTransform,
                     skeleton->joints[i]->mInvBindPose,
                     skeleton->joints[i]->pose.mSkinningMatrix);
    }
}
