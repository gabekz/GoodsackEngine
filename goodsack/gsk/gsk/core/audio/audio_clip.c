/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "audio_clip.h"

#include <stdlib.h>

#include "asset/import/loader_wav.h"
#include "util/filesystem.h"

gsk_AudioClip *
gsk_audio_clip_load_from_file(const char *uri)
{
    gsk_AudioClip *ret = malloc(sizeof(gsk_AudioClip));
    ret->p_audio_data  = gsk_load_wav(GSK_PATH(uri));

    return ret;
}