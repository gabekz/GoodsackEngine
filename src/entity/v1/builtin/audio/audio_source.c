#include "audio_source.h"

#include <core/drivers/alsoft/alsoft.h>
#include <core/drivers/alsoft/alsoft_debug.h>

#include <entity/v1/builtin/camera/camera.h>
#include <entity/v1/builtin/transform/transform.h>

static void
init(Entity e)
{
    if (!(ecs_has(e, C_AUDIOSOURCE))) return;

    struct ComponentAudioSource *audioSource = ecs_get(e, C_AUDIOSOURCE);

    audioSource->bufferId = openal_generate_source(audioSource->filePath);

    alSourcef(audioSource->bufferId, AL_REFERENCE_DISTANCE, 10.0f);
    alSourcef(audioSource->bufferId, AL_MAX_DISTANCE, 100.0f);

    audioSource->looping = 0;
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_AUDIOSOURCE))) return;
    struct ComponentAudioSource *audioSource = ecs_get(e, C_AUDIOSOURCE);

    // Update position relative to transform
    if ((ecs_has(e, C_TRANSFORM))) {
        struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
        AL_CHECK(alSource3f(audioSource->bufferId,
                            AL_POSITION,
                            transform->position[0],
                            transform->position[1],
                            transform->position[2]));
    }

    /*
    AL_CHECK(alSourcei(audioSource->bufferId, AL_LOOPING,
                audioSource->looping));
    */
}

void
s_audio_source_init(ECS *ecs)
{
    //_ECS_DECL_COMPONENT(
    //  ecs, C_AUDIO_SOURCE, sizeof(struct ComponentAudioSource));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = NULL,
                          .update  = (ECSSubscriber)update,
                        }));
}
