#ifndef H_C_AUDIO_LISTENER
#define H_C_AUDIO_LISTENER

#include <ecs/ecs.h>

struct ComponentAudioListener
{
    int a;
};

void
s_audio_listener_init(ECS *ecs);

#endif // H_C_AUDIO_LISTENER
