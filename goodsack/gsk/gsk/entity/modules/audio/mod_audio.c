/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "mod_audio.h"

#include "util/sysdefs.h"

#include "core/audio/audio_clip.h"
#include "core/drivers/alsoft/alsoft.h"
#include "core/drivers/alsoft/alsoft_buffer.h"
#include "core/drivers/alsoft/alsoft_debug.h"

void
gsk_mod_audio_set_clip(struct ComponentAudioSource *p_audio_source,
                       gsk_AudioClip *p_audio_clip)
{
    // has a buffer. Close it.
    if (p_audio_source->buffer_audio)
    {
        // stop source buffer
        AL_CHECK(alSourceStop(p_audio_source->buffer_audio));
        // detach audio buffer
        AL_CHECK(alSourcei(p_audio_source->buffer_source, AL_BUFFER, 0));
        // delete audio buffer
        openal_buffer_cleanup(p_audio_source->buffer_audio);
    }

    // create new audio buffer
    p_audio_source->buffer_audio = openal_buffer_create(p_audio_clip);

    // attach new audio buffer
    AL_CHECK(alSourcei(
      p_audio_source->buffer_source, AL_BUFFER, p_audio_source->buffer_audio));

// setup rolloff
#if 0
    AL_CHECK(alSourcef(p_audio_source->buffer_source, AL_ROLLOFF_FACTOR, 1));
    AL_CHECK(alSourcef(p_audio_source->buffer_source,
                       AL_REFERENCE_DISTANCE,
                       p_audio_source->min_distance));
    AL_CHECK(alSourcef(p_audio_source->buffer_source,
                       AL_MAX_DISTANCE,
                       p_audio_source->max_distance));
#endif

    // reset is_playing state
    p_audio_source->is_playing = FALSE;
}

void
gsk_mod_audio_play(struct ComponentAudioSource *p_audio_source)
{
    if (p_audio_source->is_playing == TRUE) { return; }

    AL_CHECK(alSourcePlay(p_audio_source->buffer_source));
    p_audio_source->is_playing = TRUE;
}

void
gsk_mod_audio_stop(struct ComponentAudioSource *p_audio_source)
{
    if (p_audio_source->is_playing == FALSE) { return; }

    AL_CHECK(alSourceStop(p_audio_source->buffer_source));
    p_audio_source->is_playing = FALSE;
}