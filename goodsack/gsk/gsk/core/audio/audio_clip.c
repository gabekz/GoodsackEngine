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

#include "asset/asset.h"
#include "asset/gpak/gpak_archive.h"

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

void
gsk_audio_clip_archive(u8 archive_mode,
                       gsk_AudioClip *p_clip,
                       gsk_AssetBlob *p_blob)
{
    gsk_Archive archive = {
      .mode = archive_mode,
    };

    // read-mode
    if (archive_mode == GskArchiveMode_Read)
    {
        archive.p_buffer = p_blob->p_buffer;
        archive.seek_cnt = 0;
    }
    // write-mode
    else if (archive_mode == GskArchiveMode_Write)
    {
        archive.out = LIST_INIT(sizeof(u8), 20);
    }

    GPAK_ARCHIVE(&archive, &p_clip->audio_data.sampleRate);
    GPAK_ARCHIVE(&archive, &p_clip->audio_data.numChannels);
    GPAK_ARCHIVE(&archive, &p_clip->audio_data.samples);

    GPAK_ARCHIVE(&archive, &p_clip->audio_data.data_size);

    if (archive_mode == GskArchiveMode_Read)
    {
        p_clip->audio_data.p_data = malloc(p_clip->audio_data.data_size);
    }

    gsk_archive_bytes(
      &archive, p_clip->audio_data.p_data, p_clip->audio_data.data_size);

    if (archive_mode == GskArchiveMode_Write)
    {
        p_blob->asset_type    = GskAssetType_Audio;
        p_blob->p_buffer      = archive.out.data.buffer;
        p_blob->buffer_len    = archive.out.data.buffer_size;
        p_blob->is_serialized = TRUE;
    }
}