#include "animation.h"

#include <core/graphics/mesh/mesh.h>

void
animation_set_keyframe(Animation *animation, ui32 keyframe)
{
    // get keyframe based on: t = delta / prev+next

    Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++) {

        Pose currentPose = skeleton->joints[i]->pose;
        Pose nextPose    = *animation->keyframes[keyframe]->poses[i];

        // Pose newPose = LERP(currentPose, newPose, t);
        Pose newPose              = nextPose;
        skeleton->joints[i]->pose = nextPose;
    }
}

void
animation_set_keyframe_lerp(Animation *animation, ui32 keyframe, float ratio)
{
    // get keyframe based on: t = delta / prev+next

    Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++) {

        Pose currentPose = skeleton->joints[i]->pose;
        Pose nextPose    = *animation->keyframes[keyframe]->poses[i];

        // Pose newPose = LERP(currentPose, newPose, t);
        Pose newPose;

        glm_vec3_lerp(currentPose.translation, nextPose.translation, ratio, newPose.translation);
        glm_vec4_lerp(currentPose.rotation, nextPose.rotation, ratio, newPose.rotation);
        glm_vec3_lerp(currentPose.scale, nextPose.scale, ratio, newPose.scale);
        skeleton->joints[i]->pose = newPose; // must be set for parent transform-correction. Maybe slow

        mat4 output = GLM_MAT4_ZERO_INIT;
        //joint_transform_world(skeleton->joints[i], (float *)output);
        glm_mat4_copy(output, skeleton->joints[i]->pose.mTransform); // set as current pose
        //glm_mat4_copy(output, animation->keyframes[keyframe]->poses[i]->mTransform);

    }
}