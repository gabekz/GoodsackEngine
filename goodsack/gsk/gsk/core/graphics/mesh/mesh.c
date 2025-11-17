/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "mesh.h"

#include "util/logger.h"
#include "util/sysdefs.h"

#include "asset/import/loader_gltf.h"
#include "asset/import/loader_obj.h"
#include "core/device/device.h"
#include "runtime/gsk_runtime_wrapper.h"

static u32 s_ordered_lengths[GSK_MESH_BUFFER_FLAGS_TOTAL] = {
  GskMeshVertexLength_Positions,
  GskMeshVertexLength_Textures,
  GskMeshVertexLength_Normals,
  GskMeshVertexLength_Tangents,
  GskMeshVertexLength_Bitangents,
  GskMeshVertexLength_Joints,
  GskMeshVertexLength_Weights,
  GskMeshVertexLength_Indices};

static gsk_MeshBuffer *
_find_buf(gsk_MeshData *d, uint32_t flag)
{
    for (int i = 0; i < d->mesh_buffers_count; i++)
    {
        if (d->mesh_buffers[i].buffer_flags & flag)
        {
            return &d->mesh_buffers[i];
        }
    }
    return NULL;
}

static u32
_resolve_vertex_id(gsk_MeshData *p_mesh_data, uint32_t draw_index)
{
    const gsk_MeshBuffer *ibo =
      _find_buf(p_mesh_data, GskMeshBufferFlag_Indices);
    if (!ibo) { return draw_index; } // non-indexed mesh

    const u64 by16  = (u64)p_mesh_data->indicesCount * sizeof(u16);
    const u8 is_u16 = (ibo->buffer_size == by16);

    if (is_u16)
    {
        const u16 *idx = (const u16 *)ibo->p_buffer;
        return (u32)idx[draw_index];
    }

    const u32 *idx = (const u32 *)ibo->p_buffer;
    return idx[draw_index];
}

static gsk_VertexAttribInfo
__get_vertex_attrib_info(gsk_MeshBuffer *p_mesh_buffer,
                         GskMeshBufferFlags vertex_flag)
{
    gsk_VertexAttribInfo ret = {0};

    for (int i = 0; i < GSK_MESH_BUFFER_FLAGS_TOTAL; i++)
    {
        s32 flag = (1 << i);
        if (vertex_flag != flag) { continue; }

        if (!(p_mesh_buffer->buffer_flags & flag)) { continue; }

        // This buffer has the flag. Get the other flags
        for (int j = 0; j < GSK_MESH_BUFFER_FLAGS_TOTAL; j++)
        {
            s32 flag_cmp = (1 << j);
            if (flag == flag_cmp) { continue; }

            if (!(p_mesh_buffer->buffer_flags & flag_cmp)) { continue; }

            if (flag_cmp < flag)
            {
                ret.spacing_before += s_ordered_lengths[j];

            } else if (flag_cmp > flag)
            {
                ret.spacing_after += s_ordered_lengths[j];
            }
        }
    }

    return ret;
}

gsk_Mesh *
gsk_mesh_allocate(gsk_MeshData *p_mesh_data)
{
    gsk_Mesh *mesh = malloc(sizeof(gsk_Mesh));
    if (mesh == NULL) { LOG_CRITICAL("Failed to allocate memory for Mesh"); }

    mesh->meshData = p_mesh_data;

    mesh->is_gpu_loaded = FALSE;

    GskMeshBufferFlags used_flags = 0;

    for (int i = 0; i < mesh->meshData->mesh_buffers_count; i++)
    {
        p_mesh_data->mesh_buffers[i].buffer_stride = 0;

        for (int j = 0; j < GSK_MESH_BUFFER_FLAGS_TOTAL; j++)
        {
            s32 flag = (1 << j);

            if (p_mesh_data->mesh_buffers[i].buffer_flags & flag)
            {
                if (used_flags & flag)
                {
                    LOG_ERROR("Duplicate mesh vertex data.");
                    return NULL;
                }

                used_flags |= flag;

                p_mesh_data->mesh_buffers[i].buffer_stride +=
                  s_ordered_lengths[j];
            }
        }

        mesh->meshData->combined_flags = used_flags;
    }

    for (int i = 0; i < GSK_MESH_BUFFER_FLAGS_TOTAL; i++)
    {
        s32 flag = (1 << i);

        gsk_MeshBuffer *pnt = _find_buf(p_mesh_data, flag);
        if (pnt == NULL) { continue; }

        pnt->vertex_attribs[i] = __get_vertex_attrib_info(pnt, flag);
    }

    return mesh;
}

u8
gsk_mesh_assemble(gsk_Mesh *mesh)
{
    if (mesh == NULL) { LOG_CRITICAL("Passing NULL Mesh to assemble."); }

    VulkanDeviceContext *p_vk_device = gsk_runtime_get_renderer()->vulkanDevice;

    if (mesh->is_gpu_loaded == TRUE)
    {
        LOG_WARN("Trying to upload an already gpu-loaded Mesh.");
        return 0;
    }

    gsk_MeshData *data = mesh->meshData;

    // GL
    gsk_GlVertexArray *vao = NULL;

    // TODO: Remove
    u8 has_vulkan_vbo = FALSE;

    if (GSK_DEVICE_API_OPENGL)
    {
        // Create the GL VAO
        vao = gsk_gl_vertex_array_create();
        gsk_gl_vertex_array_bind(vao);
        mesh->vao = vao;
    }

    GskMeshBufferFlags used_flags = 0; // overall flags of mesh

    for (int i = 0; i < data->mesh_buffers_count; i++)
    {
        // GL
        gsk_GlVertexBuffer *vbo = NULL;

        if (GSK_DEVICE_API_OPENGL)
        {
            vbo = gsk_gl_vertex_buffer_create(data->mesh_buffers[i].p_buffer,
                                              data->mesh_buffers[i].buffer_size,
                                              data->usage_draw);
        }

        for (int j = 0; j < GSK_MESH_BUFFER_FLAGS_TOTAL; j++)
        {
            s32 flag = (1 << j);

            // skip IBO for now. Done later.
            if (flag == GskMeshBufferFlag_Indices) { continue; }

            // get number of vals
            s32 n_vals = s_ordered_lengths[j];
            u32 gl_type =
              (flag == GskMeshBufferFlag_Joints) ? GL_UNSIGNED_INT : GL_FLOAT;

            if (data->mesh_buffers[i].buffer_flags & flag)
            {
                if (used_flags & flag)
                {
                    LOG_ERROR("Duplicate mesh vertex data.");
                    return 0;
                }

                if (GSK_DEVICE_API_OPENGL)
                {
                    gsk_gl_vertex_buffer_push(vbo, n_vals, gl_type, GL_FALSE);
                }

                used_flags |= flag;
            }
        }

        if (GSK_DEVICE_API_OPENGL)
        {
            gsk_gl_vertex_array_add_buffer(vao, vbo); // VBO push -> VAO
        }

        else if (GSK_DEVICE_API_VULKAN && has_vulkan_vbo == FALSE)
        {
            mesh->vkVBO =
              vulkan_vertex_buffer_create(p_vk_device->physicalDevice,
                                          p_vk_device->device,
                                          p_vk_device->graphicsQueue,
                                          p_vk_device->commandPool,
                                          data->mesh_buffers[i].p_buffer,
                                          data->mesh_buffers[i].buffer_size);

            has_vulkan_vbo = TRUE;

            if (mesh->vkVBO == NULL)
            {
                LOG_ERROR("Failed to load VK mesh buffer");
            }
        }
    }

    // Check if we have IBO
    for (int i = 0; i < data->mesh_buffers_count; i++)
    {
        if (data->mesh_buffers[i].buffer_flags & GskMeshBufferFlag_Indices)
        {
            gsk_GlIndexBuffer *ibo = NULL;

            if (GSK_DEVICE_API_OPENGL)
            {
                ibo =
                  gsk_gl_index_buffer_create(data->mesh_buffers[i].p_buffer,
                                             data->mesh_buffers[i].buffer_size,
                                             data->usage_draw);
            }

            else if (GSK_DEVICE_API_VULKAN)
            {
                mesh->vkIBO =
                  vulkan_index_buffer_create(p_vk_device->physicalDevice,
                                             p_vk_device->device,
                                             p_vk_device->graphicsQueue,
                                             p_vk_device->commandPool,
                                             data->mesh_buffers[i].p_buffer,
                                             (u16)data->indicesCount);

                if (mesh->vkVBO == NULL)
                {
                    LOG_ERROR("Failed to load VK mesh buffer");
                }
            }

            used_flags |= GskMeshBufferFlag_Indices;
        }
    }

    data->has_indices = (used_flags & GskMeshBufferFlag_Indices) ? TRUE : FALSE;
    data->isSkinnedMesh = ((used_flags & GskMeshBufferFlag_Joints) ||
                           (used_flags & GskMeshBufferFlag_Weights))
                            ? TRUE
                            : FALSE;

    data->combined_flags = used_flags;
    mesh->is_gpu_loaded  = TRUE;

    // if (GSK_DEVICE_API_VULKAN) { data->isSkinnedMesh = FALSE; }

#if 0 // USE_SKINNED_MESH
    // Skinned Mesh buffer
    gsk_GlVertexBuffer *vboSkinnedMesh; //= gsk_gl_vertex_buffer_create(data->);
    // BoneId's and weights
    gsk_gl_vertex_array_add_buffer(vao, vboSkinnedMesh);
#endif

    if (data->usage_draw == GskOglUsageType_Static)
    {
        for (int i = 0; i < data->mesh_buffers_count; i++)
        {
            if (data->mesh_buffers[i].p_buffer)
            {
                free(data->mesh_buffers[i].p_buffer);
            }
        }
    }

    // return mesh;
    return 1;
}

int
gsk_mesh_get_vertex_at_draw_index(gsk_Mesh *mesh,
                                  u32 draw_index,
                                  gsk_Vertex *out)
{
    gsk_MeshData *d = mesh->meshData;
    if (!d || !out) { return 0; }

    for (int i = 0; i < d->mesh_buffers_count; i++)
    {
        if (d->mesh_buffers[i].p_buffer == NULL) { return 0; }
    }

    const u32 vi = _resolve_vertex_id(d, draw_index);

    for (int i = 0; i < GSK_MESH_BUFFER_FLAGS_TOTAL; i++)
    {
        s32 flag            = (1 << i);
        gsk_MeshBuffer *pnt = _find_buf(d, flag);

        if (pnt == NULL) { continue; }

        const f32 *base =
          (const f32 *)pnt->p_buffer + (size_t)vi * pnt->buffer_stride;
        f32 *dest = NULL;

        u32 offset = pnt->vertex_attribs[i].spacing_before;

        switch (flag)
        {
        case GskMeshBufferFlag_Positions: dest = out->pos; break;
        case GskMeshBufferFlag_Textures: dest = out->uv; break;
        case GskMeshBufferFlag_Normals: dest = out->nrm; break;
        case GskMeshBufferFlag_Tangents: dest = out->tan; break;
        case GskMeshBufferFlag_Bitangents: dest = out->bitan; break;
        case GskMeshBufferFlag_Weights: dest = out->weights; break;
        case GskMeshBufferFlag_Joints: dest = out->joints; break;
        // TODO: should have some sort of error output here
        default: continue;
        }

        if (dest == NULL) { return 0; }

        for (int j = 0; j < s_ordered_lengths[i]; j++)
        {
            dest[j] = base[j + offset];
        }
    }

    return 1;
}
