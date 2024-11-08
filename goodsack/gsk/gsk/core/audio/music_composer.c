/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "music_composer.h"

#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/audio/audio_clip.h"
#include "core/drivers/alsoft/alsoft.h"
#include "core/drivers/alsoft/alsoft_debug.h"

static void
_sequence_create_track(gsk_ComposerSequence *p_sequence, const char *uri)
{
    gsk_AudioClip *p_clip = gsk_audio_clip_load_from_file(uri);

    s32 buffer_source = openal_generate_source();
    s32 buffer_audio  = openal_buffer_create(p_clip);

    AL_CHECK(alSourcei(buffer_source, AL_BUFFER, buffer_audio));
    // AL_CHECK(alSourcePlay(buffer_audio));

    p_sequence->num_tracks += 1;
    p_sequence->tracks[p_sequence->num_tracks] = (gsk_ComposerTrack) {
      .test = 3,
    };
}

gsk_MusicComposer
gsk_music_composer_create()
{
    gsk_MusicComposer ret = {0};
    ret.queue_next_stage  = FALSE;

    // create stages

    gsk_ComposerStage stage_1 = {
      .enter_beat = 0,
    };

    ret.stages[0] = stage_1;
    ret.total_stages++;

    // create sequences
    gsk_ComposerSequence sequence_0 = {0};
    _sequence_create_track(&sequence_0, "gsk://audio/test.wav");

    ret.sequences[0] = sequence_0;
    ret.total_sequences++;

    return ret;
}

#define BEATS_PER_BAR   4
#define BARS_PER_PHRASE 3

// handles switching stages
void
gsk_music_composer_update(gsk_MusicComposer *p_composer, double time_sec)
{
    u32 next_stage  = p_composer->current_stage + 1;
    u32 last_phrase = p_composer->current_phrase;

    f64 beat_time = 0.4511; // 133bpm

    p_composer->clock_beat_crnt = time_sec - p_composer->clock_beat_prev;

    if (p_composer->clock_beat_crnt >= beat_time)
    {
        p_composer->current_beat += 1;

        // LOG_INFO("Beat: %d", p_composer->current_beat);

        if (p_composer->current_beat == BEATS_PER_BAR)
        {
            p_composer->current_beat = 0;
            p_composer->current_bar += 1;

            // LOG_INFO("Next bar: %d", p_composer->current_bar);

            if (p_composer->current_bar == BARS_PER_PHRASE)
            {
                p_composer->current_bar = 0;
                p_composer->current_phrase += 1;

                LOG_INFO("Next phrase: %d", p_composer->current_phrase);
            }
        }

        p_composer->clock_beat_prev = time_sec;
    }

    // check to increment stage

    if (p_composer->queue_next_stage == TRUE)
    {
        if (p_composer->current_phrase != last_phrase)
        {
            p_composer->current_stage    = next_stage;
            p_composer->queue_next_stage = FALSE;
        }
    }
}

u32
gsk_music_composer_next_stage(gsk_MusicComposer *p_composer)
{
    return p_composer->current_stage;
}