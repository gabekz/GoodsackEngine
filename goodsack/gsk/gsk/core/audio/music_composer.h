/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_MUSIC_COMPOSER_H__
#define __GSK_MUSIC_COMPOSER_H__

#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_ComposerTrack
{
    u32 test;

} gsk_ComposerTrack;

typedef struct gsk_ComposerSequence
{
    gsk_ComposerTrack tracks[6];
    u32 num_tracks;

} gsk_ComposerSequence;

typedef struct gsk_ComposerStage
{
    u32 enter_beat;
} gsk_ComposerStage;

typedef struct gsk_MusicComposer
{
    gsk_ComposerStage stages[12];
    u32 total_stages;
    u32 current_stage;

    gsk_ComposerSequence sequences[12];
    u32 total_sequences;

    u8 is_playing;
    u8 queue_next_stage;

    f64 clock_beat_crnt;
    f64 clock_beat_prev;

    u32 current_phrase;
    u32 current_bar;
    u32 current_beat;

} gsk_MusicComposer;

gsk_MusicComposer
gsk_music_composer_create();

void
gsk_music_composer_update(gsk_MusicComposer *p_composer, double time_sec);

u32
gsk_music_composer_next_stage();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_MUSIC_COMPOSER_H__