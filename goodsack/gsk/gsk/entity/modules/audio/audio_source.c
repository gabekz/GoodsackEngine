/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "audio_source.h"

#include "core/drivers/alsoft/alsoft.h"
#include "core/drivers/alsoft/alsoft_debug.h"

#include "entity/modules/camera/camera.h"
#include "entity/modules/transform/transform.h"

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_AUDIOSOURCE))) return;

    struct ComponentAudioSource *audioSource = gsk_ecs_get(e, C_AUDIOSOURCE);

    audioSource->bufferId = openal_generate_source(audioSource->filePath);

    // Distance
    // AL_CHECK(alDistanceModel(AL_EXPONENT_DISTANCE)); iin LISTENER
    AL_CHECK(alSourcef(audioSource->bufferId, AL_ROLLOFF_FACTOR, 1));
    AL_CHECK(alSourcef(audioSource->bufferId, AL_REFERENCE_DISTANCE, 6));
    AL_CHECK(alSourcef(audioSource->bufferId, AL_MAX_DISTANCE, 15));

    // Play on initialization
    alSourcePlay(audioSource->bufferId);
}

static void
update(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_AUDIOSOURCE))) return;
    struct ComponentAudioSource *audioSource = gsk_ecs_get(e, C_AUDIOSOURCE);

    // Update position relative to transform
    if ((gsk_ecs_has(e, C_TRANSFORM))) {
        struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
        AL_CHECK(alSource3f(audioSource->bufferId,
                            AL_POSITION,
                            transform->position[0],
                            transform->position[1],
                            transform->position[2]));
    }

    AL_CHECK(
      alSourcei(audioSource->bufferId, AL_LOOPING, audioSource->looping));
}

void
s_audio_source_init(gsk_ECS *ecs)
{
    //_ECS_DECL_COMPONENT(
    //  ecs, C_AUDIO_SOURCE, sizeof(struct ComponentAudioSource));
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init    = (gsk_ECSSubscriber)init,
                              .destroy = NULL,
                              .render  = NULL,
                              .update  = (gsk_ECSSubscriber)update,
                            }));
}
