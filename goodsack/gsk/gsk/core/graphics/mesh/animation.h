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

typedef struct Animation Animation;
typedef struct Keyframe Keyframe;

typedef struct Joint Joint;
typedef struct Skeleton Skeleton;

typedef struct Pose Pose;

struct Pose
{
    vec3 translation;
    vec3 scale;
    vec4 rotation;

    mat4 mTransform;
    mat4 mSkinningMatrix;
    int hasMatrix;
};

struct Joint
{
    char *name;
    ui16 id;

    Joint *parent;
    ui16 childrenCount;

    Pose pose; // current pose
    mat4 mInvBindPose;
};

struct Skeleton
{
    Joint **joints;
    ui16 jointsCount;

    // GPU Buffers
    void *bufferJoints, *bufferWeights;
    ui32 bufferJointsCount, bufferWeightsCount;
    ui32 bufferJointsSize, bufferWeightsSize;

    void *skinningBuffer;
    ui32 skinningBufferSize;

    Animation *animation; // change to list

    mat4 rootMatrix;

    char *name;
};

struct Keyframe
{
    ui32 index;
    float frameTime;

    Pose **poses;
    ui32 posesCount;
};

struct Animation
{
    char *name;     // animation name
    float duration; // animation time

    Skeleton *pSkeleton; // reference to associated skeleton

    Keyframe **keyframes;
    ui32 keyframesCount;
};

// void
// animation_play(Animation *animation, ui32 index);

void
animation_set_keyframe(Animation *animation, ui32 keyframe);

void
animation_set_keyframe_lerp(Animation *animation, ui32 keyframe, float ratio);

#ifdef __cplusplus
}
#endif

#endif // __ANIMATION_H__
