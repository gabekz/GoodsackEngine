#ifndef H_MESH
#define H_MESH

#include <util/maths.h>
#include <util/sysdefs.h>

#include <core/drivers/opengl/opengl_buffer.h>
#include <core/drivers/vulkan/vulkan_vertex_buffer.h>

#include <core/graphics/mesh/animation.h>
#include <core/graphics/texture/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

// MeshData - API-agonstic buffer information
typedef struct MeshData
{
    ui32 vertexCount;
    ui32 indicesCount;
    const char *meshPath;

    ui32 totalTriangles;
    struct
    {
        // attribute buffers
        float *v, *vt, *vn; // position, texCoord, normal
        ui32 vL, vtL, vnL;  // lengths
        float *out;
        int outI;

        float *outTBN;

        ui32 *bufferIndices;
        ui32 bufferIndices_size;

    } buffers;

    // TODO: Move to model
    Skeleton *skeleton;
    int isSkinnedMesh;
} MeshData;

typedef struct Mesh
{
    VAO *vao;
    VulkanVertexBuffer *vkVBO;
    MeshData *meshData;
} Mesh;

/**
 * Assemble mesh per Graphics API spec.
 * Currently, this handles loading the model (wavefront & gltf) as well.
 *
 * @param[in] mesh path
 * @param[in] vertex scale
 * @return pointer to allocated Model structure.
 */
Mesh *
mesh_assemble(const char *path, float scale);

#ifdef __cplusplus
}
#endif

#endif // H_MESH
