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

#ifdef __cplusplus
extern "C" {
#endif

// type of BUFFER
// BUFFER_VERT (bitshift means this comes first)
//
// type of MESH
// MESH_SKINNED

// gsk_MeshData - API-agonstic buffer information
typedef struct gsk_MeshData
{
    u32 vertexCount;
    u32 indicesCount;
    u32 trianglesCount;

    u32 drawType;

    struct
    {
        // attribute buffers
        float *v, *vt, *vn; // position, texCoord, normal
        u32 vL, vtL, vnL;  // lengths

        float *out;
        u32 outI;

        float *outTBN;

        u32 *bufferIndices;
        u32 bufferIndices_size;

    } buffers;

    // TODO: Move to model
    gsk_Skeleton *skeleton;
    int isSkinnedMesh;

    // TODO: Move
    int hasTBN; // TODO: 2 == ONLY TANGENT

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
