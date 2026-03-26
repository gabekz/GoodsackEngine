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

    // gsk_model_serialize(model);

    return model;
}

gsk_AssetBlob
gsk_model_serialize(gsk_Model *p_self)
{
    gsk_AssetBlob ret = {
      .asset_type    = GskAssetType_Model,
      .is_serialized = FALSE,
      .p_buffer      = NULL,
      .buffer_len    = 0,
    };

    if (p_self == NULL)
    {
        LOG_ERROR("failed to serialize");
        return ret;
    }

    ArrayList data_list = LIST_INIT(sizeof(u8), 1000);

    LIST_APPEND(&data_list, &p_self->meshesCount, sizeof(u32));

    for (int i = 0; i < p_self->meshesCount; i++)
    {
        gsk_Mesh *p_mesh         = p_self->meshes[i];
        gsk_MeshData *p_meshdata = p_self->meshes[i]->meshData;

        // mesh local matrix
        // LIST_APPEND(&data_list, &p_mesh->localMatrix, sizeof(mat4));

        LIST_APPEND(&data_list, &p_meshdata->vertexCount, sizeof(u32));
        LIST_APPEND(&data_list, &p_meshdata->indicesCount, sizeof(u32));
        LIST_APPEND(&data_list, &p_meshdata->mesh_buffers_count, sizeof(u32));

        // primitive type, usage
        LIST_APPEND(&data_list, &p_meshdata->primitive_type, sizeof(s32));
        LIST_APPEND(
          &data_list, &p_meshdata->usage_draw, sizeof(GskOglUsageType));

        for (int j = 0; j < p_meshdata->mesh_buffers_count; j++)
        {

            gsk_MeshBuffer *p_meshbuff = &p_meshdata->mesh_buffers[j];

            LIST_APPEND(&data_list,
                        &p_meshbuff->buffer_flags,
                        sizeof(GskMeshBufferFlags));

            LIST_APPEND(
              &data_list, &p_meshbuff->total_vertex_attribs, sizeof(u32));

            LIST_APPEND(&data_list,
                        &p_meshbuff->vertex_attribs,
                        sizeof(gsk_VertexAttribInfo) *
                          p_meshbuff->total_vertex_attribs);

            LIST_APPEND(&data_list,
                        &p_meshbuff->vertex_attrib_offsets,
                        sizeof(f32) * p_meshbuff->total_vertex_attribs);

            // stride

            LIST_APPEND(&data_list, &p_meshbuff->buffer_stride, sizeof(u32));

            // buffer

            LIST_APPEND(&data_list, &p_meshbuff->buffer_size, sizeof(u32));
            LIST_APPEND(
              &data_list, p_meshbuff->p_buffer, p_meshbuff->buffer_size);
        }
    }

    ret.p_buffer      = data_list.data.buffer;
    ret.buffer_len    = data_list.data.buffer_size;
    ret.is_serialized = TRUE;

    gsk_model_deserialize_blob(&ret);

    return ret;
}

struct _BuffReader
{
    void *p_buffer;
    u32 buffer_len;
    u64 seek_cnt;
};

static void
_buff_reader_copy(struct _BuffReader *p_reader, void *p_dest, u32 data_size)
{
    memcpy(p_dest, (char *)p_reader->p_buffer + p_reader->seek_cnt, data_size);
    p_reader->seek_cnt += data_size;
}

gsk_Model
gsk_model_deserialize_blob(gsk_AssetBlob *p_blob)
{
    gsk_Model ret = {0};
    if (p_blob == NULL) { LOG_ERROR("AssetBlob is NULL"); }

    struct _BuffReader reader = {
      .p_buffer   = p_blob->p_buffer,
      .buffer_len = p_blob->buffer_len,
      .seek_cnt   = 0,
    };

    _buff_reader_copy(&reader, &ret.meshesCount, sizeof(u32));

    ret.meshes = malloc(sizeof(gsk_Mesh *) * ret.meshesCount);

    for (int i = 0; i < ret.meshesCount; i++)
    {
        gsk_MeshData *p_meshdata = malloc(sizeof(gsk_MeshData));

        // mesh local matrix
        //_buff_reader_copy(&reader, &ret.meshes[i]->localMatrix, sizeof(mat4));

        // vertex, index
        _buff_reader_copy(&reader, &p_meshdata->vertexCount, sizeof(u32));
        _buff_reader_copy(&reader, &p_meshdata->indicesCount, sizeof(u32));

        p_meshdata->trianglesCount = p_meshdata->vertexCount / 3;

        mat4 zero_root = GLM_MAT4_IDENTITY_INIT;
        glm_mat4_copy(zero_root, p_meshdata->skeleton.rootMatrix);
        p_meshdata->skeleton.jointsCount = 0;
        p_meshdata->skeleton.name        = strdup("none");

        _buff_reader_copy(
          &reader, &p_meshdata->mesh_buffers_count, sizeof(u32));

        // primitive type, usage
        _buff_reader_copy(&reader, &p_meshdata->primitive_type, sizeof(s32));
        _buff_reader_copy(
          &reader, &p_meshdata->usage_draw, sizeof(GskOglUsageType));

        for (int j = 0; j < p_meshdata->mesh_buffers_count; j++)
        {

            // buffer flags

            // total_vertex_attribs
            // vertex_attribs[]
            // vertex_attribs_offsets[]

            // buffer size
            // copy buffer

            gsk_MeshBuffer *p_meshbuff = &p_meshdata->mesh_buffers[j];
            *p_meshbuff                = (gsk_MeshBuffer) {0};

            _buff_reader_copy(
              &reader, &p_meshbuff->buffer_flags, sizeof(GskMeshBufferFlags));
            _buff_reader_copy(
              &reader, &p_meshbuff->total_vertex_attribs, sizeof(f32));
            _buff_reader_copy(&reader,
                              &p_meshbuff->vertex_attribs,
                              sizeof(gsk_VertexAttribInfo) *
                                p_meshbuff->total_vertex_attribs);
            _buff_reader_copy(&reader,
                              &p_meshbuff->vertex_attrib_offsets,
                              sizeof(f32) * p_meshbuff->total_vertex_attribs);

            // buffer stride, size, and buffer data

            _buff_reader_copy(&reader, &p_meshbuff->buffer_stride, sizeof(u32));
            _buff_reader_copy(&reader, &p_meshbuff->buffer_size, sizeof(u32));

            void *dat = malloc(p_meshbuff->buffer_size);
            _buff_reader_copy(&reader, dat, p_meshbuff->buffer_size);
            p_meshbuff->p_buffer = dat;
        }

        ret.meshes[i] = gsk_mesh_allocate(p_meshdata);

        // mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
        // glm_mat4_copy(localMatrix, ret.meshes[i]->localMatrix);

        ret.meshes[i]->usingImportedMaterial = FALSE;
    }

    return ret;
}