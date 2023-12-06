/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "animator.h"

#include "core/graphics/mesh/animation.h"
#include "entity/modules/components.h"

#include "core/device/device.h"

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

#if 0
    GLFWwindow *window = e.ecs->renderer->window;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {

        if (animator->cntTime < animator->cntAnimation->keyframesCount-1) {
            animator->cntTime++;
            gsk_animation_set_keyframe(animator->cntAnimation, animator->cntTime);
        } else {
            animator->cntTime = 0;
            gsk_animation_set_keyframe(animator->cntAnimation, 0);
        }
    }
#else

    animator->timerNow += (gsk_device_getAnalytics().delta) * 1.0;

    gsk_Animation *cntAnimation = animator->cntAnimation;
    if (animator->timerNow >= cntAnimation->duration) {
        animator->timerNow         = 0;
        animator->cntKeyframeIndex = 1;
        return;
    }

    // LOG_INFO("Timer: %.2f", animator->timerNow);

    u32 cntKeyframeIndex      = animator->cntKeyframeIndex;
    gsk_Keyframe *cntKeyframe = cntAnimation->keyframes[cntKeyframeIndex];

    u32 nxtKeyframeIndex      = cntKeyframeIndex + 1;
    gsk_Keyframe *nxtKeyframe = cntAnimation->keyframes[nxtKeyframeIndex];

    float ratio = (animator->timerNow - cntKeyframe->frameTime) /
                  (nxtKeyframe->frameTime - cntKeyframe->frameTime);

    if (ratio >= nxtKeyframe->frameTime) {
        // gsk_animation_set_keyframe(animator->cntAnimation, 0);
        gsk_animation_set_keyframe_lerp(
          animator->cntAnimation, nxtKeyframeIndex, ratio);
        animator->cntKeyframeIndex = nxtKeyframeIndex;
    }

    // evaluate whether timerNow is closer to current or next keyframe

#endif
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
