/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
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
    u8 override;
    u16 id;

    gsk_Joint *parent;
    u16 childrenCount;

    gsk_Pose pose; // current pose
    mat4 mInvBindPose;
};

struct gsk_Skeleton
{
    gsk_Joint **joints;
    u16 jointsCount;

    gsk_Animation **p_animations;
    u32 animations_count;
    u32 cnt_animation_index;

    gsk_Animation *animation; // change to list

    mat4 rootMatrix;

    char *name;
};

struct gsk_Keyframe
{
    u32 index;
    float frameTime;

    gsk_Pose **poses;
    u32 posesCount;
};

struct gsk_Animation
{
    char *name;     // animation name
    float duration; // animation time

    gsk_Skeleton *pSkeleton; // reference to associated skeleton
    u32 index;               // animation-index relative to the parent-skeleton

    gsk_Keyframe **keyframes;
    u32 keyframesCount;
};

// void
// animation_play(Animation *animation, u32 index);

void
gsk_animation_set_keyframe(gsk_Animation *animation, u32 keyframe);

void
gsk_animation_set_keyframe_lerp(gsk_Animation *animation,
                                u32 k0,
                                u32 k1,
                                float ratio);

void
gsk_skeleton_set_animation(gsk_Skeleton *p_skeleton, u32 index);

#ifdef __cplusplus
}
#endif

#endif // __ANIMATION_H__
