/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "mesh.h"

#include "util/logger.h"

#include "asset/import/loader_gltf.h"
#include "asset/import/loader_obj.h"
#include "core/device/device.h"

gsk_Mesh *
gsk_mesh_assemble(gsk_MeshData *meshData)
{
    gsk_Mesh *mesh     = malloc(sizeof(gsk_Mesh));
    mesh->meshData     = meshData;
    gsk_MeshData *data = mesh->meshData;

    if (GSK_DEVICE_API_OPENGL)
    {
        // Create the VAO
        gsk_GlVertexArray *vao = gsk_gl_vertex_array_create();
        gsk_gl_vertex_array_bind(vao);
        mesh->vao = vao;

        if (data->hasTBN == 0) // TODO: small hack to test out new MeshBuffer.
        {
            GskMeshBufferFlags used_flags = 0; // overall flags of mesh

            for (int i = 0; i < data->mesh_buffers_count; i++)
            {
                used_flags              = 0;
                gsk_GlVertexBuffer *vbo = gsk_gl_vertex_buffer_create(
                  data->mesh_buffers[i].p_buffer,
                  data->mesh_buffers[i].buffer_size);

                for (int j = 0; j < GSK_MESH_BUFFER_FLAGS_TOTAL; j++)
                {
                    s32 flag   = (1 << j);
                    s32 n_vals = (flag == GskMeshBufferFlag_Textures) ? 2 : 3;

                    // skip IBO for now. Done later.
                    if (flag == GskMeshBufferFlag_Indices) { continue; }

                    if (used_flags & flag)
                    {
                        LOG_CRITICAL("Duplicate mesh vertex data.");
                    }

                    if (data->mesh_buffers[i].buffer_flags & flag)
                    {
                        gsk_gl_vertex_buffer_push(
                          vbo, n_vals, GL_FLOAT, GL_FALSE);

                        used_flags |= flag;
                    }
                }

                gsk_gl_vertex_array_add_buffer(vao, vbo); // VBO push -> VAO
            }

            // Check if we have IBO
            for (int i = 0; i < data->mesh_buffers_count; i++)
            {
                if (data->mesh_buffers[i].buffer_flags &
                    GskMeshBufferFlag_Indices)
                {
                    gsk_GlIndexBuffer *ibo = gsk_gl_index_buffer_create(
                      data->mesh_buffers[i].p_buffer,
                      data->mesh_buffers[i].buffer_size);

                    used_flags = (used_flags | GskMeshBufferFlag_Indices);
                }
            }

            return mesh;
        }

        gsk_GlVertexBuffer *vbo = gsk_gl_vertex_buffer_create(
          data->buffers.buffer_vertices, data->buffers.buffer_vertices_size);

        // TODO: Temporarily disabled IBO for .obj extensions
        // if (data->buffers.buffer_indices != NULL && strcmp(ext, ".obj")) {
        if (data->buffers.buffer_indices_size > 0)
        {
            gsk_GlIndexBuffer *ibo = gsk_gl_index_buffer_create(
              data->buffers.buffer_indices, data->buffers.buffer_indices_size);
        }

        // Push our data into our single VBO
        if (data->buffers.vL > 0)
            gsk_gl_vertex_buffer_push(vbo, 3, GL_FLOAT, GL_FALSE);
        if (data->buffers.vtL > 0)
            gsk_gl_vertex_buffer_push(vbo, 2, GL_FLOAT, GL_FALSE);
        if (data->buffers.vnL > 0)
            gsk_gl_vertex_buffer_push(vbo, 3, GL_FLOAT, GL_FALSE);

        if (data->hasTBN == MESH_TBN_MODE_GLTF)
        { // TODO: REWORK PLEASE
            gsk_gl_vertex_buffer_push(vbo, 3, GL_FLOAT, GL_FALSE);
        }

        gsk_gl_vertex_array_add_buffer(vao, vbo); // VBO push -> VAO

        // TBN Buffer
        if (data->hasTBN == MESH_TBN_MODE_OBJ)
        {
            // TBN vertex buffer
            gsk_GlVertexBuffer *vboTBN = gsk_gl_vertex_buffer_create(
              data->buffers.buffer_tbn,
              data->trianglesCount * 3 * 2 * sizeof(GLfloat));
            gsk_gl_vertex_buffer_push(vboTBN, 3, GL_FLOAT, GL_FALSE); // tangent
            gsk_gl_vertex_buffer_push(
              vboTBN, 3, GL_FLOAT, GL_FALSE); // bitangent
            gsk_gl_vertex_array_add_buffer(vao, vboTBN);
            // free(data->buffers.buffer_tbn);
        }

#if 1

        if (data->isSkinnedMesh)
        {
            gsk_GlVertexBuffer *vboJoints = gsk_gl_vertex_buffer_create(
              data->skeleton->bufferJoints, data->skeleton->bufferJointsSize);
            gsk_gl_vertex_buffer_push(
              vboJoints, 4, GL_UNSIGNED_INT, GL_FALSE); // (affected by) joints
            gsk_gl_vertex_array_add_buffer(vao, vboJoints);

            gsk_GlVertexBuffer *vboWeights = gsk_gl_vertex_buffer_create(
              data->skeleton->bufferWeights, data->skeleton->bufferWeightsSize);
            gsk_gl_vertex_buffer_push(
              vboWeights, 4, GL_FLOAT, GL_FALSE); // associated weights
            gsk_gl_vertex_array_add_buffer(vao, vboWeights);
        }
#endif

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
