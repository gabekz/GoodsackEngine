/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PARSE_IMAGE_H__
#define __PARSE_IMAGE_H__

#include "asset/assetdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_AssetBlob
parse_image(const char *path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PARSE_IMAGE_H__