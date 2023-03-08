#include "animation.h"

#include <core/graphics/mesh/mesh.h>

#include <string.h>

static void
_joint_transform_local(Joint *joint, float *outMatrix)
{
    float *lm = outMatrix;

    if (joint->pose.hasMatrix) {
        memcpy(lm, joint->pose.mTransform, sizeof(float) * 16);
    } else {
        Pose *node = &joint->pose;
        float tx   = node->translation[0];
        float ty   = node->translation[1];
        float tz   = node->translation[2];

        float qx = node->rotation[0];
        float qy = node->rotation[1];
        float qz = node->rotation[2];
        float qw = node->rotation[3];

        float sx = node->scale[0];
        float sy = node->scale[1];
        float sz = node->scale[2];

        lm[0] = (1 - 2 * qy * qy - 2 * qz * qz) * sx;
        lm[1] = (2 * qx * qy + 2 * qz * qw) * sx;
        lm[2] = (2 * qx * qz - 2 * qy * qw) * sx;
        lm[3] = 0.f;

        lm[4] = (2 * qx * qy - 2 * qz * qw) * sy;
        lm[5] = (1 - 2 * qx * qx - 2 * qz * qz) * sy;
        lm[6] = (2 * qy * qz + 2 * qx * qw) * sy;
        lm[7] = 0.f;

        lm[8]  = (2 * qx * qz + 2 * qy * qw) * sz;
        lm[9]  = (2 * qy * qz - 2 * qx * qw) * sz;
        lm[10] = (1 - 2 * qx * qx - 2 * qy * qy) * sz;
        lm[11] = 0.f;

        lm[12] = tx;
        lm[13] = ty;
        lm[14] = tz;
        lm[15] = 1.f;
    }
}

static void
_joint_transform_world(Joint *joint, float *outMatrix)
{
    float *lm = outMatrix;
    _joint_transform_local(joint, lm);

    const Joint *parent = joint->parent;

    while (parent) {
        float pm[16];
        _joint_transform_local(parent, pm);

        for (int i = 0; i < 4; ++i) {

            float l0 = lm[i * 4 + 0];
            float l1 = lm[i * 4 + 1];
            float l2 = lm[i * 4 + 2];

            float r0 = l0 * pm[0] + l1 * pm[4] + l2 * pm[8];
            float r1 = l0 * pm[1] + l1 * pm[5] + l2 * pm[9];
            float r2 = l0 * pm[2] + l1 * pm[6] + l2 * pm[10];

            lm[i * 4 + 0] = r0;
            lm[i * 4 + 1] = r1;
            lm[i * 4 + 2] = r2;
        }

        lm[12] += pm[12];
        lm[13] += pm[13];
        lm[14] += pm[14];

        parent = parent->parent;
    }
}


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

        ui32 keyframeActual = keyframe;
        Pose currentPose = *animation->keyframes[keyframe-1]->poses[i];
        Pose nextPose    = *animation->keyframes[keyframe]->poses[i];

        // Pose newPose = LERP(currentPose, newPose, t);
        Pose newPose;

        glm_vec3_lerp(currentPose.translation, nextPose.translation, ratio, newPose.translation);
        glm_vec4_lerp(currentPose.rotation, nextPose.rotation, ratio, newPose.rotation);
        glm_vec3_lerp(currentPose.scale, nextPose.scale, ratio, newPose.scale);
        newPose.hasMatrix = 0;
        skeleton->joints[i]->pose = newPose; // must be set for parent transform-correction. Maybe slow

        mat4 output = GLM_MAT4_ZERO_INIT;
        _joint_transform_world(skeleton->joints[i], (float *)output);
        glm_mat4_copy(output, skeleton->joints[i]->pose.mTransform); // set as current pose
        //glm_mat4_copy(output, animation->keyframes[keyframe]->poses[i]->mTransform);

    }
}