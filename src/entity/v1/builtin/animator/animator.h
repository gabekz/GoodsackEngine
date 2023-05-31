#ifndef H_C_ANIMATOR
#define H_C_ANIMATOR

#include <core/graphics/mesh/animation.h>
#include <entity/v1/ecs.h>

#if !(USING_GENERATED_COMPONENTS)
struct ComponentAnimator
{
    int cntTime;
    Animation *cntAnimation;

    float timerStart, timerNow;

    int cntKeyframeIndex, nxtKeyframeIndex;
};
#endif

void
s_animator_init(ECS *ecs);

#endif // H_C_ANIMATOR
