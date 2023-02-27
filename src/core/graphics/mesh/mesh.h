#ifndef H_MESH
#define H_MESH

#include <util/sysdefs.h>

#include <core/drivers/opengl/opengl_buffer.h>
#include <core/drivers/vulkan/vulkan_vertex_buffer.h>

#include <core/graphics/texture/texture.h>

typedef struct MeshData
{
    ui32 vertexCount;
    ui32 indicesCount;
    const char *meshPath;

    ui32 totalTriangles;
    struct
    {
        float *v, *vt, *vn;
        ui32 vL, vtL, vnL;

        float *outTBN;

        float *out;
        int outI;
    } buffers;
} MeshData;

typedef struct Mesh
{
    VAO *vao;
    MeshData *meshData;

    VulkanVertexBuffer *vkVBO;
} Mesh;

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
