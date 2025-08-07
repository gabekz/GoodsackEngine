/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __QMAP_PARSE_H__
#define __QMAP_PARSE_H__

#include "asset/qmap/qmapdefs.h"
#include "core/graphics/texture/texture_set.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_QMapPlane
gsk_qmap_parse_plane_from_line(char *line, gsk_TextureSet *p_texture_set);

gsk_QMapEntityField
gsk_qmap_parse_field_from_line(char *line);

gsk_QMapContainer
gsk_qmap_parse_map_file(const char *map_path, gsk_TextureSet *p_textureset);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __QMAP_PARSE_H__