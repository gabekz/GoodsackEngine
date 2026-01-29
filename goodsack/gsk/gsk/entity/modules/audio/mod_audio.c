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
    // stop source buffer
    gsk_mod_audio_stop(p_audio_source);

    if (p_audio_source->buffer_audio)
    {
        // detach audio buffer
        AL_CHECK(alSourcei(p_audio_source->buffer_source, AL_BUFFER, 0));
    }

    // attach buffer id to audio source
    p_audio_source->buffer_audio = p_audio_clip->al_buffer_id;
    AL_CHECK(alSourcei(
      p_audio_source->buffer_source, AL_BUFFER, p_audio_source->buffer_audio));

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