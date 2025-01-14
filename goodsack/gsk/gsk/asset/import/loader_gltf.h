/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_GLTF_H__
#define __LOADER_GLTF_H__

#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/model.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_Model *
gsk_load_gltf(const char *path, int scale, int importMaterials);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_GLTF_H__
