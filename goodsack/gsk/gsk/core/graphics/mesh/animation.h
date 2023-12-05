/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#define MAX_BONES         256
#define MAX_BONE_NAME_LEN 256

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gsk_Animation gsk_Animation;
typedef struct gsk_Keyframe gsk_Keyframe;

typedef struct gsk_Joint gsk_Joint;
typedef struct gsk_Skeleton gsk_Skeleton;

typedef struct gsk_Pose gsk_Pose;

struct gsk_Pose
{
    vec3 translation;
    vec3 scale;
    vec4 rotation;

    mat4 mTransform;
    mat4 mSkinningMatrix;
    int hasMatrix;
};

struct gsk_Joint
{
    char *name;
    ui16 id;

    gsk_Joint *parent;
    ui16 childrenCount;

    gsk_Pose pose; // current pose
    mat4 mInvBindPose;
};

struct gsk_Skeleton
{
    gsk_Joint **joints;
    ui16 jointsCount;

    // GPU Buffers
    void *bufferJoints, *bufferWeights;
    ui32 bufferJointsCount, bufferWeightsCount;
    ui32 bufferJointsSize, bufferWeightsSize;

    void *skinningBuffer;
    ui32 skinningBufferSize;

    gsk_Animation *animation; // change to list

    mat4 rootMatrix;

    char *name;
};

struct gsk_Keyframe
{
    ui32 index;
    float frameTime;

    gsk_Pose **poses;
    ui32 posesCount;
};

struct gsk_Animation
{
    char *name;     // animation name
    float duration; // animation time

    gsk_Skeleton *pSkeleton; // reference to associated skeleton

    gsk_Keyframe **keyframes;
    ui32 keyframesCount;
};

// void
// animation_play(Animation *animation, ui32 index);

void
gsk_animation_set_keyframe(gsk_Animation *animation, ui32 keyframe);

void
gsk_animation_set_keyframe_lerp(gsk_Animation *animation,
                                ui32 keyframe,
                                float ratio);

#ifdef __cplusplus
}
#endif

#endif // __ANIMATION_H__
