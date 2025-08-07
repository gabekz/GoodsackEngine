/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_QMAP_H__
#define __LOADER_QMAP_H__

#include "asset/qmap/qmapdefs.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_QMapContainer
gsk_qmap_load(const char *map_path,
              gsk_TextureSet *p_texture_set,
              gsk_ShaderProgram *p_shader_program);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_QMAP_H__