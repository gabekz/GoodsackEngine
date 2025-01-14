/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_OBJ_H__
#define __LOADER_OBJ_H__

#include "core/graphics/mesh/mesh.h"

gsk_MeshData *
gsk_load_obj(const char *path, float scale);

#endif // __LOADER_OBJ_H__
