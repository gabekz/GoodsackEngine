/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __AUDIO_CLIP_H__
#define __AUDIO_CLIP_H__

#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct gsk_AudioData
{
    s32 sampleRate;
    u32 numChannels, samples, dataSize;
    u16 *data;
} gsk_AudioData;

typedef struct gsk_AudioClip
{
    gsk_AudioData *p_audio_data;
} gsk_AudioClip;

gsk_AudioClip *
gsk_audio_clip_load_from_file(const char *uri);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __AUDIO_CLIP_H__