/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "audio_source.h"

#include "core/audio/audio_clip.h"
#include "core/drivers/alsoft/alsoft.h"
#include "core/drivers/alsoft/alsoft_debug.h"

#include "entity/modules/camera/camera.h"
#include "entity/modules/transform/transform.h"
#include "gsk/core/device/device.h"

// audio module
#include "entity/modules/audio/mod_audio.h"

#include "util/filesystem.h"
#include "util/maths.h"

#define DEFAULT_MIN 8
#define DEFAULT_MAX 15

#define USING_RIGIDBODY_VELOCITY 1
#define VELOCITY_SCALE           1

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_AUDIOSOURCE))) return;

    struct ComponentAudioSource *cmp_audio_source =
      gsk_ecs_get(e, C_AUDIOSOURCE);

    cmp_audio_source->buffer_source = openal_generate_source();

    if (cmp_audio_source->audio_clip)
    {

        gsk_mod_audio_set_clip(cmp_audio_source,
                               (gsk_AudioClip *)cmp_audio_source->audio_clip);
    }

    // Distance
    // AL_CHECK(alDistanceModel(AL_EXPONENT_DISTANCE)); in LISTENER
    AL_CHECK(alSourcef(cmp_audio_source->buffer_source, AL_ROLLOFF_FACTOR, 1));

    if (!cmp_audio_source->min_distance)
    {
        cmp_audio_source->min_distance = DEFAULT_MIN;
    }
    if (!cmp_audio_source->max_distance)
    {
        cmp_audio_source->max_distance = DEFAULT_MAX;
    }

    AL_CHECK(alSourcef(cmp_audio_source->buffer_source,
                       AL_REFERENCE_DISTANCE,
                       cmp_audio_source->min_distance));
    AL_CHECK(alSourcef(cmp_audio_source->buffer_source,
                       AL_MAX_DISTANCE,
                       cmp_audio_source->max_distance));

    // Play on initialization
    if (cmp_audio_source->play_on_start == TRUE)
    {
        alSourcePlay(cmp_audio_source->buffer_source);
    }

    // default pitch to 1.0f
    if (cmp_audio_source->pitch == 0) { cmp_audio_source->pitch = 1.0f; }
}

static void
update(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_AUDIOSOURCE))) return;
    struct ComponentAudioSource *cmp_audio_source =
      gsk_ecs_get(e, C_AUDIOSOURCE);

// Update position relative to transform
#if 1
    if ((gsk_ecs_has(e, C_TRANSFORM)))
    {
        struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
        AL_CHECK(alSource3f(cmp_audio_source->buffer_source,
                            AL_POSITION,
                            transform->position[0],
                            transform->position[1],
                            transform->position[2]));
    }
#endif

#if USING_RIGIDBODY_VELOCITY
    if (gsk_ecs_has(e, C_RIGIDBODY))
    {
        struct ComponentRigidbody *cmp_rigidbody = gsk_ecs_get(e, C_RIGIDBODY);
        ALfloat sourceVelocity[]                 = {
          cmp_rigidbody->linear_velocity[0] * VELOCITY_SCALE,
          cmp_rigidbody->linear_velocity[1] * VELOCITY_SCALE,
          cmp_rigidbody->linear_velocity[2] * VELOCITY_SCALE,
        };
        AL_CHECK(alSourcefv(
          cmp_audio_source->buffer_source, AL_VELOCITY, sourceVelocity));
    }
#endif // USING_RIGIDBODY_VELOCITY

    // update looping
    AL_CHECK(alSourcei(cmp_audio_source->buffer_source,
                       AL_LOOPING,
                       cmp_audio_source->is_looping));

    // update distances
    AL_CHECK(alSourcef(cmp_audio_source->buffer_source,
                       AL_REFERENCE_DISTANCE,
                       cmp_audio_source->min_distance));
    AL_CHECK(alSourcef(cmp_audio_source->buffer_source,
                       AL_MAX_DISTANCE,
                       cmp_audio_source->max_distance));

    // update pitch
    AL_CHECK(alSourcef(
      cmp_audio_source->buffer_source, AL_PITCH, cmp_audio_source->pitch));

    // get the Audio Source state
    ALint source_state;
    AL_CHECK(alGetSourcei(
      cmp_audio_source->buffer_source, AL_SOURCE_STATE, &source_state));

    // set is_playing by state
    cmp_audio_source->is_playing = (source_state == AL_PLAYING) ? TRUE : FALSE;
}

static void
destroy(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_AUDIOSOURCE))) return;
    // LOG_INFO("Destroy");
}

void
s_audio_source_init(gsk_ECS *ecs)
{
    //_ECS_DECL_COMPONENT(
    //  ecs, C_AUDIO_SOURCE, sizeof(struct ComponentAudioSource));
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init    = (gsk_ECSSubscriber)init,
                              .update  = (gsk_ECSSubscriber)update,
                              .destroy = (gsk_ECSSubscriber)destroy,
                            }));
}
