#ifndef H_C_AUDIO_SOURCE
#define H_C_AUDIO_SOURCE

#include <ecs/ecs.h>

#include <core/api/openal_soft/openal.h>

struct ComponentAudioSource
{
    ALuint bufferId;
    const char *filePath;

    int looping;
};

void
s_audio_source_init(ECS *ecs);

#endif // H_C_AUDIO_LISTENER
