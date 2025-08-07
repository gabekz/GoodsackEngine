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

#define BEATS_PER_BAR   4
#define BARS_PER_PHRASE 4
#define TEMPO_MS        0.5 // 120bpm

static void
_sequence_create_track(gsk_ComposerSequence *p_sequence, const char *uri)
{
    gsk_AudioClip *p_clip = gsk_audio_clip_load_from_file(uri);

    s32 buffer_source = openal_generate_source();
    s32 buffer_clip   = openal_buffer_create(p_clip);

    AL_CHECK(alSourcei(buffer_source, AL_BUFFER, buffer_clip));
    AL_CHECK(alSourcei(buffer_source, AL_LOOPING, TRUE));

    AL_CHECK(alSourcef(buffer_source, AL_ROLLOFF_FACTOR, 0));

    // AL_CHECK(alSourcef(buffer_source, AL_GAIN, 0.1f));

    p_sequence->tracks[p_sequence->num_tracks] = (gsk_ComposerTrack) {
      .buffer_source = buffer_source,
      .buffer_clip   = buffer_clip,
      .is_looping    = TRUE,
      .is_playing    = FALSE,
    };

    p_sequence->num_tracks += 1;
}

static void
_play_track(gsk_MusicComposer *p_composer, gsk_ComposerTrack *p_track)
{
    ALint source_state = 0;
    AL_CHECK(
      alGetSourcei(p_track->buffer_source, AL_SOURCE_STATE, &source_state));

    p_track->is_playing = (source_state == AL_PLAYING) ? TRUE : FALSE;

    if (p_track->is_playing == TRUE) { return; }

    AL_CHECK(alSourcef(
      p_track->buffer_source, AL_SEC_OFFSET, p_composer->time_offset));

    AL_CHECK(alSourcePlay(p_track->buffer_clip));
}

static void
_begin_stage(gsk_MusicComposer *p_composer, u32 next_stage)
{
    // go to next stage
    gsk_ComposerStage *p_next_stage = &p_composer->stages[next_stage];

    gsk_ComposerTrack *p_playlist[32] = {0};
    u32 playlist_total                = 0;

    for (int i = 0; i < p_next_stage->sequence_indices_total; i++)
    {
        gsk_ComposerSequence *p_sequence =
          &p_composer->sequences[p_next_stage->sequence_indices[i]];

        for (int j = 0; j < p_sequence->num_tracks; j++)
        {
            gsk_ComposerTrack *p_track = &p_sequence->tracks[j];
            _play_track(p_composer, p_track);

            p_playlist[playlist_total] = p_track;
            playlist_total++;
        }
    }

    // initial stage.
    if (next_stage == 0) { return; };

    gsk_ComposerStage *p_last_stage = &p_composer->stages[next_stage - 1];
    for (int i = 0; i < p_last_stage->sequence_indices_total; i++)
    {
        gsk_ComposerSequence *p_sequence =
          &p_composer->sequences[p_last_stage->sequence_indices[i]];

        for (int j = 0; j < p_sequence->num_tracks; j++)
        {
            gsk_ComposerTrack *p_track = &p_sequence->tracks[j];
            u8 skip_stop               = FALSE;

            // TODO: Maky a sequence playlist, not a track playlist.
            // we want to check by sequence.

            for (int k = 0; k < playlist_total; k++)
            {
                if (p_track == p_playlist[k])
                {
                    LOG_TRACE("Track is included, don't stop");
                    skip_stop = TRUE;
                    break;
                }
            }

            if (skip_stop == FALSE)
            {
                AL_CHECK(alSourceStop(p_track->buffer_clip));
            }
        }
    }
}

gsk_MusicComposer
gsk_music_composer_create()
{
    gsk_MusicComposer ret = {0};
    ret.queue_next_stage  = TRUE;

    // create stages

    gsk_ComposerStage stage_1 = {
      .enter_beat             = 0,
      .sequence_indices_total = 0,
    };

    ret.stages[0] = stage_1;
    ret.total_stages++;

    gsk_ComposerStage stage_2 = {
      .enter_beat             = 0,
      .sequence_indices[0]    = 0,
      .sequence_indices_total = 1,
    };

    ret.stages[1] = stage_2;
    ret.total_stages++;

    gsk_ComposerStage stage_3 = {
      .enter_beat             = 0,
      .sequence_indices[0]    = 0,
      .sequence_indices[1]    = 1,
      .sequence_indices_total = 2,
    };

    ret.stages[2] = stage_3;
    ret.total_stages++;

    // create sequences
    gsk_ComposerSequence sequence_0 = {0};
    _sequence_create_track(&sequence_0, "gsk://audio/pattern_1.wav");

    ret.sequences[0] = sequence_0;
    ret.total_sequences++;

    gsk_ComposerSequence sequence_1 = {0};
    _sequence_create_track(&sequence_1, "gsk://audio/pattern_2.wav");

    ret.sequences[1] = sequence_1;
    ret.total_sequences++;

    _begin_stage(&ret, 0);

    return ret;
}

// handles switching stages
void
gsk_music_composer_update(gsk_MusicComposer *p_composer, double time_sec)
{
    u32 next_stage  = p_composer->current_stage + 1;
    u32 last_phrase = p_composer->current_phrase;

    // f64 beat_time = 0.4511; // 133bpm
    f64 beat_time = TEMPO_MS;

    p_composer->clock_beat_crnt = time_sec - p_composer->clock_beat_prev;
    p_composer->time_offset     = p_composer->clock_beat_crnt - beat_time;

    if (p_composer->clock_beat_crnt >= beat_time)
    {
        p_composer->current_beat += 1;

        // LOG_INFO("Beat: %d", p_composer->current_beat);

        if (p_composer->current_beat == BEATS_PER_BAR)
        {
            p_composer->current_beat = 0;
            p_composer->current_bar += 1;

            LOG_TRACE("next bar: %d", p_composer->current_bar);

            if (p_composer->current_bar == BARS_PER_PHRASE)
            {
                p_composer->current_bar = 0;
                p_composer->current_phrase += 1;

                LOG_DEBUG("next phrase: %d", p_composer->current_phrase);
            }
        }

        p_composer->clock_beat_prev = time_sec;
    }

    // check to increment stage

    if (p_composer->queue_next_stage == TRUE &&
        next_stage < p_composer->total_stages)
    {
        if (p_composer->current_phrase != last_phrase)
        {
            LOG_DEBUG("setting next stage (due to end of phrase) from %d to %d",
                      last_phrase,
                      p_composer->current_phrase);

            p_composer->current_stage = next_stage;
            // p_composer->queue_next_stage = FALSE;

            _begin_stage(p_composer, next_stage);
        }
    }
}

u32
gsk_music_composer_next_stage(gsk_MusicComposer *p_composer)
{
    return p_composer->current_stage;
}