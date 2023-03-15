#ifndef H_MESH
#define H_MESH

#include <util/maths.h>
#include <util/sysdefs.h>

#include <core/drivers/opengl/opengl_buffer.h>
#include <core/drivers/vulkan/vulkan_vertex_buffer.h>

#include <core/graphics/mesh/animation.h>
#include <core/graphics/texture/texture.h>

#define DRAW_ARRAYS   0x00
#define DRAW_ELEMENTS 0x01

#ifdef __cplusplus
extern "C" {
#endif

// type of BUFFER
// BUFFER_VERT (bitshift means this comes first)
//
// type of MESH
// MESH_SKINNED

// MeshData - API-agonstic buffer information
typedef struct MeshData
{
    ui32 vertexCount;
    ui32 indicesCount;
    ui32 trianglesCount;

    ui32 drawType;

    struct
    {
        // attribute buffers
        float *v, *vt, *vn; // position, texCoord, normal
        ui32 vL, vtL, vnL;  // lengths

        float *out;
        ui32 outI;

        float *outTBN;

        ui32 *bufferIndices;
        ui32 bufferIndices_size;

    } buffers;

    // TODO: Move to model
    Skeleton *skeleton;
    int isSkinnedMesh;

    // TODO: Move
    int hasTBN;

} MeshData;

typedef struct Mesh
{
    VAO *vao;
    VulkanVertexBuffer *vkVBO;
    MeshData *meshData;
    mat4 localMatrix;
} Mesh;

/**
 * Assemble mesh per Graphics API spec.
 * Currently, this handles loading the model (wavefront & gltf) as well.
 *
 * @param[in] mesh data
 * @return pointer to allocated Model structure.
 */
Mesh *
mesh_assemble(MeshData *meshData);

#ifdef __cplusplus
}
#endif

#endif // H_MESH
