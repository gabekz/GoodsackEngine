/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__

#include "core/graphics/mesh/animation.h"
#include "entity/v1/ecs.h"

#if !(USING_GENERATED_COMPONENTS)
struct ComponentAnimator
{
    int cntTime;
    gsk_Animation *cntAnimation;

    float timerStart, timerNow;

    int cntKeyframeIndex, nxtKeyframeIndex;
};
#endif

void
s_animator_init(gsk_ECS *ecs);

#endif // __ANIMATOR_H__
