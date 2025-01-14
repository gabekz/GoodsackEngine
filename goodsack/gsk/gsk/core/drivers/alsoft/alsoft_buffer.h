/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ALSOFT_BUFFER_H__
#define __ALSOFT_BUFFER_H__

#include "core/audio/audio_clip.h"
#include "util/sysdefs.h"

// TODO: Move to thirdparty directive - gkutuzov/GoodsackEngine#19
#include <AL/al.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Creates an OpenAL audio buffer and fills the data immediately
 * by loading the specified .wav file.
 *
 * @param[in] p_audio_clip - pointer to already loaded audio clip
 * @return buffer ID (ALuint)
 */
ALuint
openal_buffer_create(gsk_AudioClip *p_audio_clip);

/**
 * Deletes the specified OpenAL audio buffer
 *
 * @param[in] buffer Id
 */
void
openal_buffer_cleanup(ALuint buffer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ALSOFT_BUFFER_H__