/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_ASSET_FONT_H__
#define __GSK_ASSET_FONT_H__

#include "core/graphics/texture/texture.h"
#include "util/sysdefs.h"

#define GSK_FONT_MAX_ATLAS_CHARS 256

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_Font
{
    gsk_Texture *p_texture; // font atlas texture
    u8 char_spacing[GSK_FONT_MAX_ATLAS_CHARS];

    ivec2 sheet_size;
    ivec2 cell_size;

    u32 sprite_rows;
    u32 sprite_cols;

    gsk_AssetBlob image_blob;
} gsk_Font;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_ASSET_FONT_H__
