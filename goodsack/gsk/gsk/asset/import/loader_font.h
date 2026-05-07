/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_LOADER_FONT_H__
#define __GSK_LOADER_FONT_H__

#include "asset/asset_font.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_Font
gsk_font_import_from_file(const char *path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__GSK_LOADER_FONT_H__