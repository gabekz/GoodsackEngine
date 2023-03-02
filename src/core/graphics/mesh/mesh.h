#ifndef H_MESH
#define H_MESH

#include <util/sysdefs.h>

#include <core/drivers/opengl/opengl_buffer.h>
#include <core/drivers/vulkan/vulkan_vertex_buffer.h>

#include <core/graphics/texture/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Mesh Mesh;
typedef struct MeshData MeshData;

typedef struct Joint Joint;
typedef struct Skeleton Skeleton;

#define MAX_BONES 256

struct Joint
{
    Joint *parent;
    //Joint **children;
    ui16 childrenCount;

    char *name;
    ui16 id;
    mat4 jointMatrix;
};

struct Skeleton
{
    Joint **joints;
    ui16 jointsCount;

    // GPU Buffers
    void *bufferJoints, *bufferWeights;
    ui32 bufferJointsCount, bufferWeightsCount;
    ui32 bufferJointsSize, bufferWeightsSize;

    void *skinningBuffer;
    ui32 skinningBufferSize;
};


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

    // TODO: Move to model
    Skeleton *skeleton;
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

#ifdef __cplusplus
}
#endif

#endif // H_MESH
