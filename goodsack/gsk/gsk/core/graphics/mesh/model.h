/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MODEL_H__
#define __MODEL_H__

#include "core/graphics/mesh/animation.h"
#include "core/graphics/mesh/mesh.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum ModelFileType { OBJ = 0, GLTF, QMAP } ModelFileType;

#if 0
typedef struct ModelOptions
{
    float scale;
    u16 import_materials;
} TextureOptions;
#endif

typedef struct gsk_Model
{
    const char *modelPath;

    gsk_Mesh **meshes;
    u32 meshesCount;

    // TODO: should have skeleton here, AnimationComponent should have
    // AnimationSet reference.
    // OR the AnimationComponent should just have it all..
    // gsk_Skeleton *p_skeleton;

    ModelFileType fileType;

} gsk_Model;

gsk_Model *
gsk_model_load_from_file(const char *path, f32 scale, u16 importMaterials);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MODEL_H__
