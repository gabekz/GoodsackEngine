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
    u32 numChannels;
    u32 samples;
    u32 data_size;
    void *p_data;
} gsk_AudioData;

typedef struct gsk_AudioClip
{
    gsk_AudioData audio_data;
    u32 al_buffer_id;
} gsk_AudioClip;

gsk_AudioClip
gsk_audio_clip_import_from_file(const char *uri);

u8
gsk_audio_clip_load(gsk_AudioClip *p_self);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __AUDIO_CLIP_H__