#ifndef H_MESH
#define H_MESH

#include <util/sysdefs.h>

#include <core/drivers/opengl/opengl_buffer.h>
#include <core/drivers/vulkan/vulkan_vertex_buffer.h>

#include <core/graphics/texture/texture.h>

typedef struct Mesh Mesh;
typedef struct MeshData MeshData;

struct Mesh
{
    VAO *vao;
    VulkanVertexBuffer *vkVBO;
    MeshData *meshData;
};

// MeshData - API-agonstic buffer information
struct MeshData
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
};

/**
 * Assemble mesh per Graphics API spec.
 * Currently, this handles loading the model (.obj) as well.
 *
 * @param[in] mesh path
 * @param[in] vertex scale
 * @return pointer to allocated Model structure.
 */
Mesh *
mesh_assemble(const char *path, float scale);

#endif // H_MESH
