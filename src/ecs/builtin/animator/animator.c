#include "animator.h"

#include <core/graphics/mesh/animation.h>
#include <ecs/builtin/components.h>

#include <core/device/device.h>

static void
init(Entity e)
{
    if (!(ecs_has(e, C_ANIMATOR))) return;
    if (!(ecs_has(e, C_MODEL))) return;

    struct ComponentAnimator *animator = ecs_get(e, C_ANIMATOR);
    struct ComponentModel *model = ecs_get(e, C_MODEL);

    Skeleton *skeleton = model->mesh->meshData->skeleton;
    Animation *animation = skeleton->animation;

    animation_set_keyframe(animation, 0);

    animator->cntAnimation = animation;
    animator->cntTime      = 0;

    animator->timerNow   = animator->timerStart;

    animator->cntKeyframeIndex = 0;
    animator->nxtKeyframeIndex = 1;
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_ANIMATOR))) return;
    if (!(ecs_has(e, C_MODEL))) return;

    struct ComponentAnimator *animator = ecs_get(e, C_ANIMATOR);
    struct ComponentModel *model = ecs_get(e, C_MODEL);

#if 0
    GLFWwindow *window = e.ecs->renderer->window;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {

        if (animator->cntTime < animator->cntAnimation->keyframesCount-1) {
            animator->cntTime++;
            animation_set_keyframe(animator->cntAnimation, animator->cntTime);
        } else {
            animator->cntTime = 0;
            animation_set_keyframe(animator->cntAnimation, 0);
        }
    }
#else

    animator->timerNow += (device_getAnalytics().delta);

    if (animator->timerNow >= animator->cntAnimation->duration) {
        animator->timerNow = 0;

#if 1   // for the sake of testing -> going to set keyframe to 1
        animator->cntKeyframeIndex = 1;
#else
        animator->cntKeyframeIndex = 0;
#endif
        return;
    }

    //LOG_INFO("Timer: %.2f", animator->timerNow);

    ui32 cntKeyframeIndex = animator->cntKeyframeIndex;
    Keyframe *cntKeyframe = animator->cntAnimation->keyframes[cntKeyframeIndex];

    ui32 nxtKeyframeIndex = cntKeyframeIndex + 1;
    Keyframe *nxtKeyframe = animator->cntAnimation->keyframes[nxtKeyframeIndex];

    // evaluate ratio - TODO: this is not used yet
    float ratio = (animator->timerNow - cntKeyframe->frameTime) /
                  (nxtKeyframe->frameTime - cntKeyframe->frameTime);
    //LOG_INFO("Ratio is: %f", ratio);

    if (ratio >= nxtKeyframe->frameTime) {
        //animation_set_keyframe(animator->cntAnimation, 0);
        animation_set_keyframe_lerp(animator->cntAnimation, nxtKeyframeIndex, ratio);
        animator->cntKeyframeIndex = nxtKeyframeIndex;
    }

    // evaluate whether timerNow is closer to current or next keyframe

#endif


}

void
s_animator_init(ECS *ecs)
{
    ecs_component_register(
      ecs, C_ANIMATOR, sizeof(struct ComponentAnimator));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = NULL,
                          .update  = (ECSSubscriber)update,
                        }));
}
