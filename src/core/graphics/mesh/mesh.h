#ifndef H_MESH
#define H_MESH

#include <util/maths.h>
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

typedef struct Pose Pose;
typedef struct Animation Animation;
typedef struct Keyframe Keyframe;

#define MAX_BONES         256
#define MAX_BONE_NAME_LEN 256

struct Pose
{
    vec3 translation;
    vec3 scale;
    vec4 rotation;

    mat4 mTransform;
    int hasMatrix;
};

struct Joint
{
    char *name;
    ui16 id;

    Joint *parent;
    ui16 childrenCount;

    Pose pose; // current pose
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

    Animation *animation; // change to list

    char *name;
};

struct Keyframe
{
    ui32 index;
    float frameTime;

    Pose **poses;
    ui32 posesCount;
};

struct Animation
{
    char *name;    // animation name
    float duration; // animation time

    Skeleton *pSkeleton; // reference to associated skeleton

    Keyframe **keyframes;
    ui32 keyframesCount;
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
    int isSkinnedMesh;
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
