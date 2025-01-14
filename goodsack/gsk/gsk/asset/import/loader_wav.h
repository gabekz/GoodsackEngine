/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_WAV_H__
#define __LOADER_WAV_H__

#include "core/audio/audio_clip.h"
#include "util/sysdefs.h"

#define SAMPLING_RATE 44100

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// WAV file loader.
// @return struct gsk_AudioData
gsk_AudioData *
gsk_load_wav(const char *filepath);

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#endif // __LOADER_WAV_H__
