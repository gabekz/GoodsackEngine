/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "model.h"

#include <string.h>

#include "util/array_list.h"
#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/graphics/mesh/mesh.h"

#include "asset/gpak/gpak_archive.h"
#include "asset/import/loader_gltf.h"
#include "asset/import/loader_obj.h"

gsk_Model *
gsk_model_load_from_file(const char *path, f32 scale, u16 importMaterials)
{
    char *ext = strrchr(path, '.');
    if (!ext)
    {
        LOG_ERROR("Failed to find file extension for %s\n", path);
        return NULL;
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

void
gsk_model_archive(GskArchiveMode archive_mode,
                  gsk_Model *p_model,
                  gsk_AssetBlob *p_blob)
{
    gsk_Archive archive = {
      .mode = archive_mode,
    };

    // read-mode
    if (archive_mode == GskArchiveMode_Read)
    {
        archive.p_buffer = p_blob->p_buffer;
        archive.seek_cnt = 0;
        LOG_DEBUG("Model READ");
    }
    // write-mode
    else if (archive_mode == GskArchiveMode_Write)
    {
        archive.out = LIST_INIT(sizeof(u8), 20);
        LOG_DEBUG("Model WRITE");
    }

    GPAK_ARCHIVE(&archive, &p_model->meshesCount);

    if (archive.mode == GskArchiveMode_Read)
    {
        p_model->meshes = malloc(sizeof(gsk_Mesh *) * p_model->meshesCount);
    }

    for (int i = 0; i < p_model->meshesCount; i++)
    {
        LOG_INFO("mesh %d", i);
        gsk_MeshData *p_meshdata = NULL;
        if (archive.mode == GskArchiveMode_Read)
        {
            p_meshdata = malloc(sizeof(gsk_MeshData));
        } else
        {
            p_meshdata = p_model->meshes[i]->meshData;
        }

        GPAK_ARCHIVE(&archive, &p_meshdata->vertexCount);
        GPAK_ARCHIVE(&archive, &p_meshdata->indicesCount);

        GPAK_ARCHIVE(&archive, &p_meshdata->boundingBox);

        // TODO: triangles count

        GPAK_ARCHIVE(&archive, &p_meshdata->mesh_buffers_count);

        GPAK_ARCHIVE(&archive, &p_meshdata->primitive_type);
        GPAK_ARCHIVE(&archive, &p_meshdata->usage_draw);

        GPAK_ARCHIVE(&archive, &p_meshdata->isSkinnedMesh);
        GPAK_ARCHIVE(&archive, &p_meshdata->animations.animations_count);
        // if (p_meshdata->isSkinnedMesh) { LOG_INFO("is_skinned"); }

        for (int j = 0; j < p_meshdata->mesh_buffers_count; j++)
        {
            gsk_MeshBuffer *p_meshbuff = &p_meshdata->mesh_buffers[j];
            //*p_meshbuff                = (gsk_MeshBuffer) {0};

            GPAK_ARCHIVE(&archive, &p_meshbuff->buffer_flags);
            GPAK_ARCHIVE(&archive, &p_meshbuff->total_vertex_attribs);

            gsk_archive_bytes(&archive,
                              p_meshbuff->vertex_attribs,
                              sizeof(gsk_VertexAttribInfo) *
                                p_meshbuff->total_vertex_attribs);

            gsk_archive_bytes(&archive,
                              p_meshbuff->vertex_attrib_offsets,
                              sizeof(f32) * p_meshbuff->total_vertex_attribs);

            GPAK_ARCHIVE(&archive, &p_meshbuff->buffer_stride);
            GPAK_ARCHIVE(&archive, &p_meshbuff->buffer_size);

            if (archive_mode == GskArchiveMode_Read)
            {
                p_meshbuff->p_buffer = malloc(p_meshbuff->buffer_size);
            }
            gsk_archive_bytes(
              &archive, p_meshbuff->p_buffer, p_meshbuff->buffer_size);
        }

        GPAK_ARCHIVE(&archive, &p_meshdata->skeleton.jointsCount);
        GPAK_ARCHIVE(&archive, &p_meshdata->skeleton.rootMatrix);

#if 0
        if (p_meshdata->isSkinnedMesh)
        {
            s32 str_len = strlen(p_meshdata->skeleton.name);
            GPAK_ARCHIVE(&archive, &str_len);
            gsk_archive_bytes(&archive, p_meshdata->skeleton.name, str_len);
        }
#endif

        if (archive_mode == GskArchiveMode_Read)
        {
            p_model->meshes[i] = gsk_mesh_allocate(p_meshdata);
            p_model->meshes[i]->usingImportedMaterial = FALSE;
        }
    }

    if (archive_mode == GskArchiveMode_Write)
    {
        p_blob->asset_type    = GskAssetType_Model;
        p_blob->p_buffer      = archive.out.data.buffer;
        p_blob->buffer_len    = archive.out.data.buffer_size;
        p_blob->is_serialized = TRUE;
    }
}