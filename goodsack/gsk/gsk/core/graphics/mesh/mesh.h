/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
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

// TODO: Rework
#define MESH_TBN_MODE_NONE 0
#define MESH_TBN_MODE_OBJ  1
#define MESH_TBN_MODE_GLTF 2

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
    u8 has_indices;
    u8 hasTBN; // TODO: 2 == ONLY TANGENT

    struct
    {
        u32 vL, vtL, vnL; // buffer sets

        float *buffer_vertices; // Position, Texture, Normal
        float *buffer_indices;  // indices
        float *buffer_tbn;      // Tangent, Bitangent

        u32 buffer_vertices_size;
        u32 buffer_indices_size;
        u32 buffer_tbn_size;
    } buffers;

    gsk_MeshBuffer mesh_buffers[4];
    u32 mesh_buffers_count;

    // TODO: Move to model
    gsk_Skeleton *skeleton;
    int isSkinnedMesh;

    vec3 boundingBox[2];

} gsk_MeshData;

typedef struct gsk_Mesh
{
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
 * Assemble mesh per Graphics API spec.
 * Currently, this handles loading the model (wavefront & gltf) as well.
 *
 * @param[in] mesh data
 * @return pointer to allocated Model structure.
 */
gsk_Mesh *
gsk_mesh_assemble(gsk_MeshData *meshData);

#ifdef __cplusplus
}
#endif

#endif // __MESH_H__
