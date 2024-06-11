/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_QMAP_H__
#define __LOADER_QMAP_H__

#include "asset/qmap/qmapdefs.h"
#include "core/graphics/material/material.h"
#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//#define QMAP_TYPE 0

gsk_QMapContainer
gsk_qmap_load(const char *map_path, gsk_TextureSet *p_texture_set);

#if 0
void *
gsk_qmap_attach_textures(gsk_QMapContainer *p_container, void *p_texture_set);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_QMAP_H__