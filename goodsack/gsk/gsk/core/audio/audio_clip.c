/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "audio_clip.h"

#include <stdlib.h>

#include "asset/import/loader_wav.h"
#include "core/drivers/alsoft/alsoft.h"
#include "util/filesystem.h"
#include "util/logger.h"

gsk_AudioClip
gsk_audio_clip_import_from_file(const char *uri)
{
    gsk_AudioClip ret = {0};
    ret.audio_data    = gsk_load_wav(GSK_PATH(uri));

    return ret;
}

u8
gsk_audio_clip_load(gsk_AudioClip *p_self)
{
    if (p_self == NULL)
    {
        LOG_ERROR("Failed to load audio: gsk_AudioClip is NULL");
        return 0;
    }

    if (p_self->audio_data.p_data == NULL)
    {
        LOG_ERROR("Failed to load audio: audio_data is NULL");
        return 0;
    }

    p_self->al_buffer_id = openal_buffer_create(&p_self->audio_data);

    return 1;
}