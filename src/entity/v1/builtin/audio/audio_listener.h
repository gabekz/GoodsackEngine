#ifndef H_C_AUDIO_LISTENER
#define H_C_AUDIO_LISTENER

#include <entity/v1/ecs.h>

#if !(USING_GENERATED_COMPONENTS)
struct ComponentAudioListener
{
    int a;
};
#endif

void
s_audio_listener_init(ECS *ecs);

#endif // H_C_AUDIO_LISTENER
