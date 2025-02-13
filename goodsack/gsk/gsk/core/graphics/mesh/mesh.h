/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MESH_H__
#define __MESH_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/drivers/opengl/opengl_buffer.h"
#include "core/drivers/vulkan/vulkan_vertex_buffer.h"

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/animation.h"
#include "core/graphics/texture/texture.h"

#define DRAW_ARRAYS             0x00
#define DRAW_ELEMENTS           0x01
#define DRAW_ELEMENTS_WIREFRAME 0x02

#ifdef __cplusplus
extern "C" {
#endif

typedef enum GskMeshPrimitiveType_ {
    GskMeshPrimitiveType_Triangle,
    GskMeshPrimitiveType_Quad,
    GskMeshPrimitiveType_Poly,
    GskMeshPrimitiveType_Fan,
} GskMeshPrimitiveType_;

typedef enum GskMeshBufferFlag_ {
    GskMeshBufferFlag_Positions  = 1 << 0,
    GskMeshBufferFlag_Textures   = 1 << 1,
    GskMeshBufferFlag_Normals    = 1 << 2,
    GskMeshBufferFlag_Tangents   = 1 << 3,
    GskMeshBufferFlag_Bitangents = 1 << 4,
    GskMeshBufferFlag_Joints     = 1 << 5,
    GskMeshBufferFlag_Weights    = 1 << 6,
    GskMeshBufferFlag_Indices    = 1 << 7,
} GskMeshBufferFlag_;
#define GSK_MESH_BUFFER_FLAGS_TOTAL 8

typedef enum GskMeshVertexLength_ {
    GskMeshVertexLength_Positions  = 3,
    GskMeshVertexLength_Textures   = 2,
    GskMeshVertexLength_Normals    = 3,
    GskMeshVertexLength_Tangents   = 3,
    GskMeshVertexLength_Bitangents = 3,
    GskMeshVertexLength_Joints     = 4,
    GskMeshVertexLength_Weights    = 4,
    GskMeshVertexLength_Indices    = 1
} GskMeshVertexLength_;

typedef s32 GskMeshBufferFlags;

typedef struct gsk_MeshBuffer
{
    float *p_buffer;
    u32 buffer_size;
    GskMeshBufferFlags buffer_flags;

} gsk_MeshBuffer;

// gsk_MeshData - API-agonstic buffer information
typedef struct gsk_MeshData
{
    u32 vertexCount;
    u32 indicesCount;
    u32 trianglesCount;

    GskMeshPrimitiveType_ primitive_type;
    GskOglUsageType usage_draw;
    u8 has_indices;
    u8 isSkinnedMesh;

    gsk_MeshBuffer mesh_buffers[4];
    u32 mesh_buffers_count;
    GskMeshBufferFlags combined_flags; // flags used by every buff

    // TODO: Move to model
    gsk_Skeleton *skeleton;

    vec3 boundingBox[2];
    vec3 world_pos;

} gsk_MeshData;

typedef struct gsk_Mesh
{
    u8 is_gpu_loaded;

    // Mesh data
    gsk_MeshData *meshData;
    mat4 localMatrix;

    // Mesh GPU buffers
    gsk_GlVertexArray *vao;
    VulkanVertexBuffer *vkVBO;

    // Imported material data
    u32 usingImportedMaterial;
    gsk_Material *materialImported;
} gsk_Mesh;

/**
 * Setup Mesh for GPU readiness.
 *
 * @return pointer to heap-allocated Mesh.
 */
gsk_Mesh *
gsk_mesh_allocate(gsk_MeshData *p_mesh_data);

/**
 * Upload mesh to GPU per Graphics API spec.
 *
 * @param[in] mesh data
 * @return pointer to mesh
 */
u8
gsk_mesh_assemble(gsk_Mesh *mesh);

#ifdef __cplusplus
}
#endif

#endif // __MESH_H__
