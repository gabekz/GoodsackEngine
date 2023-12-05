/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "alsoft_buffer.h"

#include "asset/import/loader_wav.h"
#include "core/drivers/alsoft/alsoft_debug.h"

/* static */

static ALenum
FileToEnum(ui16 channels, ui16 samples)
{
    int stereo = (channels > 1);

    switch (samples) {
    case 16: return (stereo) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    case 8: return (stereo) ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
    default: return -1;
    }
}

ALuint
openal_buffer_create(const char *filePath)
{
    // Load .wav file
    gsk_AudioData *rawData = gsk_load_wav(filePath);
    ALenum format      = FileToEnum(rawData->numChannels, rawData->samples);

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
