/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MOD_AUDIO_H__
#define __MOD_AUDIO_H__

#include "util/sysdefs.h"

#include "core/audio/audio_clip.h"
#include "entity/ecs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void
gsk_mod_audio_set_clip(struct ComponentAudioSource *p_audio_source,
                       gsk_AudioClip *p_audio_clip);

void
gsk_mod_audio_play(struct ComponentAudioSource *p_audio_source);

void
gsk_mod_audio_stop(struct ComponentAudioSource *p_audio_source);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MOD_AUDIO_H__
