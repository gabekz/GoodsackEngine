#ifndef H_MODEL
#define H_MODEL

#include <util/sysdefs.h>

#include <core/api/opengl/glbuffer.h>
#include <core/api/vulkan/vulkan_vertex_buffer.h>

#include <core/texture/texture.h>

typedef struct _model Model;
typedef struct _modelData ModelData;

struct _model
{
    VAO *vao;
    ModelData *modelData;

    VulkanVertexBuffer *vkVBO;
};

struct _modelData
{
    ui32 vertexCount;
    ui32 indicesCount;
    const char *modelPath;

    ui32 totalTriangles;
    struct
    {
        float *v, *vt, *vn;
        ui32 vL, vtL, vnL;

        float *outTBN;

        float *out;
        int outI;
    } buffers;
};

/**
 * Assemble model per Graphics API spec.
 * Currently, this handles loading the model (.obj) as well.
 *
 * @param[in] model path
 * @param[in] vertex scale
 * @return pointer to allocated Model structure.
 */
Model *
model_assemble(const char *path, float scale);

#endif // H_MODEL
