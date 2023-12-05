/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_WAV_H__
#define __LOADER_WAV_H__

#include "util/sysdefs.h"

#define SAMPLING_RATE 44100

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_AudioData
{
    si32 sampleRate;
    ui32 numChannels, samples, dataSize;
    ui16 *data;
} gsk_AudioData;

// WAV file loader.
// @return struct gsk_AudioData
gsk_AudioData *
gsk_load_wav(const char *filepath);

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#endif // __LOADER_WAV_H__
