/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MODEL_H__
#define __MODEL_H__

#include "util/sysdefs.h"
#include "core/graphics/mesh/mesh.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum ModelFileType { OBJ = 0, GLTF } ModelFileType;

#if 0
typedef struct ModelOptions
{
    float scale;
    ui16 import_materials;
} TextureOptions;
#endif

typedef struct Model
{

    const char *modelPath;

    Mesh **meshes;
    ui32 meshesCount;

    ModelFileType fileType;

} Model;

Model *
model_load_from_file(const char *path, f32 scale, ui16 importMaterials);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MODEL_H__
