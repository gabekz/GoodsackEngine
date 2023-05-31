#ifndef H_C_AUDIO_SOURCE
#define H_C_AUDIO_SOURCE

#include <entity/v1/ecs.h>

#include <core/drivers/alsoft/alsoft.h>

#if !(USING_GENERATED_COMPONENTS)
typedef struct ComponentAudioSource
{
    ALuint bufferId;
    const char *filePath;

    int looping;
} ComponentAudioSource;
#endif

void
s_audio_source_init(ECS *ecs);

#endif // H_C_AUDIO_LISTENER
