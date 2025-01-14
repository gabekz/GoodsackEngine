/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "animator.h"

#include "core/device/device.h"
#include "core/graphics/mesh/animation.h"
#include "core/graphics/mesh/mesh.h"

#include "entity/__generated__/components_gen.h"

#define ANIMATOR_REWIND 0 // TODO: Move to component as a setting

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_ANIMATOR))) return;
    if (!(gsk_ecs_has(e, C_MODEL))) return;

    struct ComponentAnimator *animator = gsk_ecs_get(e, C_ANIMATOR);
    struct ComponentModel *model       = gsk_ecs_get(e, C_MODEL);

    gsk_Mesh *mesh = model->mesh;

    if (!mesh->meshData->isSkinnedMesh)
    {
        _gsk_ecs_set_internal(e, C_ANIMATOR, FALSE);
        return;
    }

    gsk_Skeleton *skeleton   = mesh->meshData->skeleton;
    gsk_Animation *animation = skeleton->animation;

    gsk_animation_set_keyframe(animation, 0);

    animator->cntAnimation = animation;
    animator->cntTime      = 0;

    animator->timerNow = animator->timerStart;

    animator->cntKeyframeIndex = 0;
    animator->nxtKeyframeIndex = 1;

    animator->is_transition_delayed = TRUE;
    animator->is_looping            = FALSE;
    animator->is_playing            = TRUE;

    animator->force_replay = FALSE;
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

    if (animator->is_playing)
    {
        animator->timerNow +=
          (gsk_device_getTime().delta_time) * gsk_device_getTime().time_scale;
    }

    gsk_Animation *cntAnimation = animator->cntAnimation;
    u32 cntKeyframeIndex        = animator->cntKeyframeIndex;
    u32 nxtKeyframeIndex        = cntKeyframeIndex + 1;

    u32 cnt_anim_index = cntAnimation->index;

    // if (nxtKeyframeIndex >= cntAnimation->keyframesCount &&
    //    !animator->is_looping) {
    //    animator->is_playing = FALSE;
    //}

    // check if we want to have a delay in animation transition

    u16 has_anim_changed =
      (cnt_anim_index != cntAnimation->pSkeleton->cnt_animation_index);

    // switched + delayed
    u16 delay_ready = (!animator->is_transition_delayed && has_anim_changed);

    // switched + delayed + (looping + not playing)
    u16 has_anim_ended = (delay_ready) ||
                         (animator->is_looping && !animator->is_playing) ||
                         (has_anim_changed && !animator->is_playing);

    // NOTE: At this point, we probably don't need to check the timer.
    // Just ensure that the keyframe doesn't go out-of-bounds. That is
    // a lot more practical than a timing check..
    if (animator->timerNow >= cntAnimation->duration ||
        nxtKeyframeIndex > cntAnimation->keyframesCount - 1 || has_anim_ended ||
        animator->force_replay)
    {
        animator->timerNow         = 0;
        animator->cntKeyframeIndex = 1;

        animator->is_playing =
          (animator->force_replay) || (animator->is_looping || 0);
        animator->force_replay = FALSE;

        // potentially update animation. NOTE: Important to leave this here -
        // we only want to switch animations if we are done with the current
        // one.
        // kind of cursed.
        animator->cntAnimation = cntAnimation->pSkeleton->animation;

#if ANIMATOR_REWIND
        gsk_animation_set_keyframe(animator->cntAnimation, 1);
#endif // ANIMATOR_REWIND

        return;
    }

    // if (!animator->is_playing) return;

    gsk_Keyframe *cntKeyframe = cntAnimation->keyframes[cntKeyframeIndex];
    gsk_Keyframe *nxtKeyframe = cntAnimation->keyframes[nxtKeyframeIndex];

    float ratio = (animator->timerNow - cntKeyframe->frameTime) /
                  (nxtKeyframe->frameTime - cntKeyframe->frameTime);

    if (ratio >= nxtKeyframe->frameTime)
    {
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
