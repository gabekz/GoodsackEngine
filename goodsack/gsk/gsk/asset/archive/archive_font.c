/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "archive_font.h"

#include "asset/asset_font.h"
#include "asset/assetdefs.h"
#include "asset/gpak/gpak_archive.h"

#include "util/sysdefs.h"

void
gsk_asset_archive_font(GskArchiveMode archive_mode,
                       gsk_Font *p_font,
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
        LOG_DEBUG("Font READ");
    }
    // write-mode
    else if (archive_mode == GskArchiveMode_Write)
    {
        archive.out = LIST_INIT(sizeof(u8), 20);
        LOG_DEBUG("Font WRITE");
    }

    GPAK_ARCHIVE(&archive, &p_font->sheet_size);
    GPAK_ARCHIVE(&archive, &p_font->cell_size);

    gsk_archive_bytes(
      &archive, p_font->char_spacing, sizeof(u8) * GSK_FONT_MAX_ATLAS_CHARS);

    GPAK_ARCHIVE(&archive, &p_font->image_blob.buffer_len);

    if (archive_mode == GskArchiveMode_Read)
    {
        p_font->image_blob.p_buffer = malloc(p_font->image_blob.buffer_len);
    }

    gsk_archive_bytes(
      &archive, p_font->image_blob.p_buffer, p_font->image_blob.buffer_len);

    if (archive_mode == GskArchiveMode_Write)
    {
        p_blob->asset_type    = GskAssetType_Model;
        p_blob->p_buffer      = archive.out.data.buffer;
        p_blob->buffer_len    = archive.out.data.buffer_size;
        p_blob->is_serialized = TRUE;
    }
}