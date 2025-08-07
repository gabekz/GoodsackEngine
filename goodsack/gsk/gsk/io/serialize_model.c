/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "serialize_model.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset/assetdefs.h"

#include "util/logger.h"

#pragma pack(push, 1)
struct _MeshDataHeader
{
    u32 vert_count;
    u32 triangle_count;
};
#pragma pack(pop)

gsk_AssetBlob
serialize_model(gsk_Model *p_model)
{
    gsk_AssetBlob ret = {0};

    // TODO: does not work for >1 meshes
    for (int i = 0; i < p_model->meshesCount; i++)
    {
        gsk_MeshData *p_meshdata = p_model->meshes[i]->meshData;

        // construct header data //

        struct _MeshDataHeader header = {
          .vert_count     = p_meshdata->vertexCount,
          .triangle_count = p_meshdata->trianglesCount,
        };

        u32 header_size = (sizeof(struct _MeshDataHeader));

        // fill buffer data //

        u32 total_buffers           = p_meshdata->mesh_buffers_count;
        u32 total_buffers_size      = 0;
        GskMeshBufferFlags flags[4] = {0};

        // go through all buffers
        for (int j = 0; j < total_buffers_size; j++)
        {
            total_buffers_size += p_meshdata->mesh_buffers[j].buffer_size;
            flags[j] = p_meshdata->mesh_buffers[j].buffer_flags;
        }

        // allocate buffer //

        ret.buffer_len = header_size + total_buffers_size;
        ret.p_buffer   = malloc(ret.buffer_len);

        if (ret.p_buffer == NULL)
        {
            LOG_ERROR("Failed to allocate buffer of size %d", ret.buffer_len);
            return ret;
        }

        // writing  to buffer //

        // write header
        memcpy(ret.p_buffer, &header, header_size);

        // write meshdata-buffers
        u32 last_pos = header_size;
        for (int j = 0; j < total_buffers_size; j++)
        {
            u32 newadd = p_meshdata->mesh_buffers[j].buffer_size;
            memcpy((char *)ret.p_buffer + last_pos,
                   p_meshdata->mesh_buffers[j].p_buffer,
                   newadd);
            last_pos += newadd;
        }
    }

    return ret;
}

gsk_Model
extract_model(gsk_AssetBlob *p_blob)
{
    gsk_Model ret = {0};
}