/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "mesh.h"

#include "util/logger.h"

#include "asset/import/loader_gltf.h"
#include "asset/import/loader_obj.h"
#include "core/device/device.h"

gsk_Mesh *
gsk_mesh_allocate(gsk_MeshData *p_mesh_data)
{
    gsk_Mesh *mesh = malloc(sizeof(gsk_Mesh));
    if (mesh == NULL) { LOG_CRITICAL("Failed to allocate memory for Mesh"); }

    mesh->meshData      = p_mesh_data;
    mesh->is_gpu_loaded = FALSE;

    GskMeshBufferFlags used_flags = 0;

    for (int i = 0; i < mesh->meshData->mesh_buffers_count; i++)
    {
        for (int j = 0; j < GSK_MESH_BUFFER_FLAGS_TOTAL; j++)
        {
            s32 flag = (1 << j);

            if (p_mesh_data->mesh_buffers[i].buffer_flags & flag)
            {
                if (used_flags & flag)
                {
                    LOG_ERROR("Duplicate mesh vertex data.");
                }

                used_flags |= flag;
            }
        }

        mesh->meshData->combined_flags = used_flags;
    }

    return mesh;
}

gsk_Mesh *
gsk_mesh_assemble(gsk_Mesh *mesh)
{
    if (mesh == NULL) { LOG_CRITICAL("Passing NULL Mesh to assemble."); }

    if (mesh->is_gpu_loaded == TRUE)
    {
        LOG_WARN("Trying to upload an already gpu uploaded Mesh.");
        return gsk_Mesh;
    }

    gsk_MeshData *data = mesh->meshData;

    if (GSK_DEVICE_API_OPENGL)
    {
        // Create the VAO
        gsk_GlVertexArray *vao = gsk_gl_vertex_array_create();
        gsk_gl_vertex_array_bind(vao);
        mesh->vao = vao;

        GskMeshBufferFlags used_flags = 0; // overall flags of mesh

        for (int i = 0; i < data->mesh_buffers_count; i++)
        {
            gsk_GlVertexBuffer *vbo =
              gsk_gl_vertex_buffer_create(data->mesh_buffers[i].p_buffer,
                                          data->mesh_buffers[i].buffer_size);

            for (int j = 0; j < GSK_MESH_BUFFER_FLAGS_TOTAL; j++)
            {
                s32 flag = (1 << j);

                // get number of vals
                s32 n_vals  = 3;
                u32 gl_type = GL_FLOAT;

                if (flag == GskMeshBufferFlag_Textures)
                {
                    n_vals = 2;
                } else if ((flag == GskMeshBufferFlag_Joints) ||
                           (flag == GskMeshBufferFlag_Weights))
                {
                    n_vals = 4;
                }

                gl_type = (flag == GskMeshBufferFlag_Joints) ? GL_UNSIGNED_INT
                                                             : GL_FLOAT;

                // skip IBO for now. Done later.
                if (flag == GskMeshBufferFlag_Indices) { continue; }

                if (data->mesh_buffers[i].buffer_flags & flag)
                {
                    if (used_flags & flag)
                    {
                        LOG_CRITICAL("Duplicate mesh vertex data.");
                    }

                    gsk_gl_vertex_buffer_push(vbo, n_vals, gl_type, GL_FALSE);

                    used_flags |= flag;
                }
            }

            gsk_gl_vertex_array_add_buffer(vao, vbo); // VBO push -> VAO
        }

        // Check if we have IBO
        for (int i = 0; i < data->mesh_buffers_count; i++)
        {
            if (data->mesh_buffers[i].buffer_flags & GskMeshBufferFlag_Indices)
            {
                gsk_GlIndexBuffer *ibo =
                  gsk_gl_index_buffer_create(data->mesh_buffers[i].p_buffer,
                                             data->mesh_buffers[i].buffer_size);

                used_flags |= GskMeshBufferFlag_Indices;
            }
        }

        data->has_indices   = (used_flags & GskMeshBufferFlag_Indices);
        data->isSkinnedMesh = ((used_flags & GskMeshBufferFlag_Joints) ||
                               (used_flags & GskMeshBufferFlag_Weights));

        // data->combined_flags = used_flags;
        mesh->is_gpu_loaded = TRUE;
        return mesh;

    } else if (GSK_DEVICE_API_VULKAN)
    {
        LOG_WARN("gsk_mesh_assemble() not implemented for Vulkan!");
    }

#if 0 // USE_SKINNED_MESH
    // Skinned Mesh buffer
    gsk_GlVertexBuffer *vboSkinnedMesh; //= gsk_gl_vertex_buffer_create(data->);
    // BoneId's and weights
    gsk_gl_vertex_array_add_buffer(vao, vboSkinnedMesh);
#endif

    return mesh;
}
