/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef _GSK_ASSET_ARCHIVE_FONT_H__
#define _GSK_ASSET_ARCHIVE_FONT_H__

#include "asset/asset_font.h"
#include "asset/gpak/gpak_archive.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void
gsk_asset_archive_font(GskArchiveMode archive_mode,
                       gsk_Font *p_font,
                       gsk_AssetBlob *p_blob);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GSK_ASSET_ARCHIVE_FONT_H__