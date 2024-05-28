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
        // AL_CHECK(alSourceStop(p_audio_source->bufferId));
        openal_buffer_cleanup(p_audio_source->buffer_audio);
    }

    p_audio_source->buffer_audio = openal_buffer_create(p_audio_clip);
    AL_CHECK(alSourcei(
      p_audio_source->buffer_source, AL_BUFFER, p_audio_source->buffer_audio));
}

void
gsk_mod_audio_play(struct ComponentAudioSource *p_audio_source)
{
    AL_CHECK(alSourcePlay(p_audio_source->buffer_source));
}

void
gsk_mod_audio_stop(struct ComponentAudioSource *p_audio_source)
{
    AL_CHECK(alSourceStop(p_audio_source->buffer_source));
}