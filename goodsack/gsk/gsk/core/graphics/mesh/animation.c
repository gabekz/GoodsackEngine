#include "animation.h"

#include <core/graphics/mesh/mesh.h>
#include <core/graphics/mesh/mesh_helpers.inl>

#include <string.h>

void
animation_set_keyframe(Animation *animation, ui32 keyframe)
{
    Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++) {

        Pose currentPose = skeleton->joints[i]->pose;
        Pose nextPose    = *animation->keyframes[keyframe]->poses[i];

        // Pose newPose = LERP(currentPose, newPose, t);
        Pose newPose              = nextPose;
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
animation_set_keyframe_lerp(Animation *animation, ui32 keyframe, float ratio)
{
    Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++) {

        ui32 keyframeActual = keyframe;
        Pose currentPose    = *animation->keyframes[keyframe - 1]->poses[i];
        Pose nextPose       = *animation->keyframes[keyframe]->poses[i];

        Pose newPose;

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