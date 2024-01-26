/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "animator.h"

#include "core/device/device.h"
#include "core/graphics/mesh/animation.h"
#include "core/graphics/mesh/mesh.h"

#include "entity/__generated__/components_gen.h"

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_ANIMATOR))) return;
    if (!(gsk_ecs_has(e, C_MODEL))) return;

    struct ComponentAnimator *animator = gsk_ecs_get(e, C_ANIMATOR);
    struct ComponentModel *model       = gsk_ecs_get(e, C_MODEL);

    gsk_Mesh *mesh = model->mesh;

    if (!mesh->meshData->isSkinnedMesh) return;

    gsk_Skeleton *skeleton   = mesh->meshData->skeleton;
    gsk_Animation *animation = skeleton->animation;

    gsk_animation_set_keyframe(animation, 0);

    animator->cntAnimation = animation;
    animator->cntTime      = 0;

    animator->timerNow = animator->timerStart;

    animator->cntKeyframeIndex = 0;
    animator->nxtKeyframeIndex = 1;
}

static void
update(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_ANIMATOR))) return;
    if (!(gsk_ecs_has(e, C_MODEL))) return;

    struct ComponentAnimator *animator = gsk_ecs_get(e, C_ANIMATOR);
    struct ComponentModel *model       = gsk_ecs_get(e, C_MODEL);
    gsk_Mesh *mesh                     = model->mesh;

    if (!mesh->meshData->isSkinnedMesh) return;

    animator->timerNow += (gsk_device_getTime().delta_time) * 1.0;

    gsk_Animation *cntAnimation = animator->cntAnimation;
    u32 cntKeyframeIndex        = animator->cntKeyframeIndex;
    u32 nxtKeyframeIndex        = cntKeyframeIndex + 1;

    // gsk_animation_set_keyframe(cntAnimation, 10);
    // return;

    // NOTE: At this point, we probably don't need to check the timer.
    // Just ensure that the keyframe doesn't go out-of-bounds. That is
    // a lot more practical than a timing check..
    if (animator->timerNow >= cntAnimation->duration ||
        nxtKeyframeIndex > cntAnimation->keyframesCount - 1) {
        animator->timerNow         = 0;
        animator->cntKeyframeIndex = 1;

        // potentially update animation. NOTE: Important to leave this here -
        // we only want to switch animations if we are done with the current
        // one.
        // kind of cursed.
        animator->cntAnimation = cntAnimation->pSkeleton->animation;

        return;
    }

    gsk_Keyframe *cntKeyframe = cntAnimation->keyframes[cntKeyframeIndex];
    gsk_Keyframe *nxtKeyframe = cntAnimation->keyframes[nxtKeyframeIndex];

    float ratio = (animator->timerNow - cntKeyframe->frameTime) /
                  (nxtKeyframe->frameTime - cntKeyframe->frameTime);

    if (ratio >= nxtKeyframe->frameTime) {
        gsk_animation_set_keyframe_lerp(
          animator->cntAnimation, nxtKeyframeIndex, ratio);
        animator->cntKeyframeIndex = nxtKeyframeIndex;
    }
}

void
s_animator_init(gsk_ECS *ecs)
{
    //_ECS_DECL_COMPONENT(ecs, C_ANIMATOR, sizeof(struct ComponentAnimator));
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init    = (gsk_ECSSubscriber)init,
                              .destroy = NULL,
                              .render  = NULL,
                              .update  = (gsk_ECSSubscriber)update,
                            }));
}
