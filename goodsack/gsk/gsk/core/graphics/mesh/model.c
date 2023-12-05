/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "model.h"

#include <string.h>

#include "util/logger.h"
#include "util/sysdefs.h"
#include "util/maths.h"

#include "core/graphics/mesh/mesh.h"

#include "asset/import/loader_gltf.h"
#include "asset/import/loader_obj.h"

Model *
model_load_from_file(const char *path, f32 scale, ui16 importMaterials)
{
    char *ext = strrchr(path, '.');
    if (!ext) {
        LOG_CRITICAL("Failed to find file extension for %s\n", path);
    } else {
        LOG_INFO("extension is %s\n", ext);
    }

    Model *model;
    // Check file extension
    if (!strcmp(ext, ".obj")) {
        model           = malloc(sizeof(Model));
        MeshData *mesh0 = load_obj(path, 1.0f); // always importing with
                                                // scale 1.0 here

        model->meshes      = malloc(sizeof(Mesh *) * 1);
        model->meshes[0]   = mesh_assemble(mesh0);
        model->modelPath   = path;
        model->meshesCount = 1;

        mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
        glm_mat4_copy(localMatrix, model->meshes[0]->localMatrix);
        model->meshes[0]->usingImportedMaterial = FALSE;

        // model->fileType = OBJ;
    } else if (!strcmp(ext, ".gltf") || !strcmp(ext, ".glb")) {
        model = load_gltf(path, scale, importMaterials);
        // model->fileType = GLTF;
    }
    return model;
}
