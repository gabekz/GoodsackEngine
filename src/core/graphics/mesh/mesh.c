#include "mesh.h"

#include <asset/import/loader_gltf.h>
#include <asset/import/loader_obj.h>
#include <core/device/device.h>
#include <util/logger.h>

#include <string.h>

Mesh *
mesh_assemble(const char *path, float scale)
{
    Mesh *mesh = malloc(sizeof(Mesh));

    // TODO: This is temporary. File discovery should not be handled here.
    char *ext = strrchr(path, '.');
    if (!ext) {
        LOG_CRITICAL("Failed to find file extension for %s\n", path);
    } else {
        LOG_INFO("extension is %s\n", ext);
    }

    // Check file extension
    if (!strcmp(ext, ".obj")) {
        mesh->meshData = load_obj(path, scale);
    } else if (!strcmp(ext, ".gltf") || !strcmp(ext, ".glb")) {
        mesh->meshData = load_gltf(path, scale);
    }

    MeshData *data = mesh->meshData;

    if (DEVICE_API_OPENGL) {
        // Create the VAO
        VAO *vao = vao_create();
        vao_bind(vao);
        mesh->vao = vao;

        VBO *vbo =
          // vbo_create(data->buffers.out, data->buffers.outI * sizeof(float));
          vbo_create(data->buffers.out, data->buffers.outI);

        // TODO: Temporarily disabled IBO for .obj extensions
        if (data->buffers.bufferIndices != NULL && strcmp(ext, ".obj")) {
            IBO *ibo = ibo_create(data->buffers.bufferIndices,
                                  data->buffers.bufferIndices_size);
        }

        // Push our data into our single VBO
        if (data->buffers.vL > 0) vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
        if (data->buffers.vtL > 0) vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
        if (data->buffers.vnL > 0) vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
        vao_add_buffer(vao, vbo); // VBO push -> VAO

#if 0
        // TBN vertex buffer
        VBO *vboTBN = vbo_create(
          data->buffers.outTBN, data->totalTriangles * 3 * 2 * sizeof(GLfloat));
        vbo_push(vboTBN, 3, GL_FLOAT, GL_FALSE); // tangent
        vbo_push(vboTBN, 3, GL_FLOAT, GL_FALSE); // bitangent
        vao_add_buffer(vao, vboTBN);
        // free(data->buffers.outTBN);
#endif

#if 1

        if (data->isSkinnedMesh) {
            VBO *vboJoints = vbo_create(data->skeleton->bufferJoints,
                                        data->skeleton->bufferJointsSize);
            vbo_push(
              vboJoints, 4, GL_UNSIGNED_INT, GL_FALSE); // (affected by) joints
            vao_add_buffer(vao, vboJoints);

            VBO *vboWeights = vbo_create(data->skeleton->bufferWeights,
                                         data->skeleton->bufferWeightsSize);
            vbo_push(vboWeights, 4, GL_FLOAT, GL_FALSE); // associated weights
            vao_add_buffer(vao, vboWeights);
            /*
            VBO *vboSkeleton = vbo_create(data->skeleton->skinningBuffer,
                                          data->skeleton->skinningBufferSize);

            vbo_push(vboSkeleton, 4, GL_UNSIGNED_INT, GL_FALSE); // (affected
            by) joints vbo_push(vboSkeleton, 4, GL_FLOAT, GL_FALSE); //
            associated weights

            vao_add_buffer(vao, vboSkeleton);
            */
        }
#endif

    } else if (DEVICE_API_VULKAN) {
        LOG_WARN("mesh_assemble() not implemented for Vulkan!");
    }

#if 0 // USE_SKINNED_MESH
    // Skinned Mesh buffer
    VBO *vboSkinnedMesh; //= vbo_create(data->);
    // BoneId's and weights
    vao_add_buffer(vao, vboSkinnedMesh);
#endif

    return mesh;
}
