/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "model.h"

#include <string.h>

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/graphics/mesh/mesh.h"

#include "asset/import/loader_gltf.h"
#include "asset/import/loader_obj.h"

gsk_Model *
gsk_model_load_from_file(const char *path, f32 scale, u16 importMaterials)
{
    char *ext = strrchr(path, '.');
    if (!ext)
    {
        LOG_CRITICAL("Failed to find file extension for %s\n", path);
    } else
    {
        LOG_INFO("extension is %s\n", ext);
    }

    gsk_Model *model;
    // Check file extension
    if (!strcmp(ext, ".obj"))
    {
        model               = malloc(sizeof(gsk_Model));
        gsk_MeshData *mesh0 = gsk_load_obj(path, scale);

        model->modelPath   = path;
        model->meshesCount = 1;
        model->meshes      = malloc(sizeof(gsk_Mesh *) * model->meshesCount);
        model->meshes[0]   = gsk_mesh_allocate(mesh0);
        model->meshes[0]->usingImportedMaterial = FALSE;

        mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
        glm_mat4_copy(localMatrix, model->meshes[0]->localMatrix);

        // model->fileType = OBJ;
    } else if (!strcmp(ext, ".gltf") || !strcmp(ext, ".glb"))
    {
        model = gsk_load_gltf(path, scale, importMaterials);
        // model->fileType = GLTF;
    }
    return model;
}
