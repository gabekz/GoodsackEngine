#ifndef H_C_ANIMATOR
#define H_C_ANIMATOR

#include <core/graphics/mesh/animation.h>
#include <ecs/ecs.h>

struct ComponentAnimator
{
    int cntTime;
    Animation *cntAnimation;

    float timerStart, timerNow;

    int cntKeyframeIndex, nxtKeyframeIndex;
};

void
s_animator_init(ECS *ecs);

#endif // H_C_ANIMATOR
