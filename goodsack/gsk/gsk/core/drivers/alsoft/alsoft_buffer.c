/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "alsoft_buffer.h"

#include "asset/import/loader_wav.h"
#include "core/audio/audio_clip.h"
#include "core/drivers/alsoft/alsoft_debug.h"

/* static */

static ALenum
FileToEnum(u16 channels, u16 samples)
{
    int stereo = (channels > 1);

    switch (samples)
    {
    case 16: return (stereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    case 8: return (stereo) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
    default: return (stereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    }
}

ALuint
openal_buffer_create(gsk_AudioClip *p_audio_clip)
{
    // Load .wav file
    gsk_AudioData *rawData = p_audio_clip->p_audio_data;
    ALenum format          = FileToEnum(rawData->numChannels, rawData->samples);

    ALuint buffer;
    AL_CHECK(alGenBuffers((ALuint)1, &buffer));
    AL_CHECK(alBufferData(
      buffer, format, rawData->data, rawData->dataSize, rawData->sampleRate));

    return buffer;
}

void
openal_buffer_cleanup(ALuint buffer)
{
    AL_CHECK(alDeleteBuffers(1, &buffer));
}
