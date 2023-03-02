#include "loader_gltf.h"
#pragma optimize("", off)

#include <core/graphics/mesh/mesh.h>
#include <util/logger.h>
#include <util/maths.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define LOGGING_GLTF

MeshData *
load_gltf(const char *path, int scale)
{
    cgltf_options options = {0};
    cgltf_data *data      = NULL;

    cgltf_result result = cgltf_parse_file(&options, path, &data);
    result              = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) {
        LOG_ERROR("Failed to load GLTF file");
    }

    int indicesBufferViewIndex = 0;

#ifdef LOGGING_GLTF
    for (int i = 0; i < data->meshes_count; i++) {
        LOG_INFO("Mesh name: %s", data->meshes[i].name);

        int attributesCount = data->meshes[i].primitives->attributes_count;
        LOG_INFO("Total attributes: %d", attributesCount);

        for (int j = 0; j < attributesCount; j++) {
            cgltf_attribute attribute =
              data->meshes[i].primitives->attributes[j];
            LOG_INFO("%d\tAttribute: %s - buffer index: %d",
                     j,
                     attribute.name,
                     attribute.index);
            indicesBufferViewIndex++;
        }

        cgltf_accessor *indices = data->meshes[i].primitives->indices;

        cgltf_size indicesSize =
          data->accessors[indicesBufferViewIndex].count *
          sizeof(
            unsigned short); // cgltf_accessor_read_index(data->accessors, 3);

        // LOG_INFO("\nIndices count: %d\tbuffer size: %d", indices->count,
        // data->accessors[indicesBufferViewIndex].count * sizeof(unsigned
        // short));
        LOG_INFO(
          "\nIndices count: %d\tbuffer size: %d", indices->count, indicesSize);

        int accessorsCount = data->accessors_count;
        LOG_INFO("\nAccessors count: %d", accessorsCount);
        for (int i = 0; i < accessorsCount; i++) {
            cgltf_accessor accessor = data->accessors[i];
            LOG_INFO("Accessor: %d\tBufferView size: %d",
                     i,
                     accessor.buffer_view->size);
        }
    }
#endif // LOGGING_GLTF

    MeshData *ret = malloc(sizeof(MeshData));

    // TEMPORARY testing

    ret->buffers.out = malloc(1794 * sizeof(float) * sizeof(vec3));
    cgltf_accessor_unpack_floats(
      data->accessors,
      ret->buffers.out,
      1794 * sizeof(vec3)); // UNPACK for out -> read_float for individual

    // total size of output
    ret->buffers.outI = 1794 * sizeof(float) * sizeof(vec3);

    ret->vertexCount = 1794;

    // set this so we push the position to buffer
    ret->buffers.vL  = 1794;
    ret->buffers.vtL = 0;
    ret->buffers.vnL = 0;
    // ret->buffers.vnL = 0;

    // Indices //

    ret->indicesCount = data->accessors[indicesBufferViewIndex].count;
    ret->buffers.bufferIndices_size = ret->indicesCount * sizeof(ui32);

    ret->buffers.bufferIndices = malloc(ret->buffers.bufferIndices_size);

    // store all indices
    for (int i = 0; i < ret->indicesCount; i++) {
        if (!cgltf_accessor_read_uint(&data->accessors[indicesBufferViewIndex],
                                      i,
                                      ret->buffers.bufferIndices + i,
                                      ret->indicesCount)) {
            LOG_ERROR("Failed to read uint! %d", i);
        }
    }
    // Joints and Weights //

    // ret->joints= data->accessors[indicesBufferViewIndex].count;
    // ret->buffers.bufferIndices_size = ret->indicesCount * sizeof(ui32);

    // ret->buffers.bufferIndices = malloc(ret->buffers.bufferIndices_size);

    // buffers
    ui32 jointsBufferSize  = 1794 * 4 * sizeof(ui32);
    ui32 weightsBufferSize = 1794 * 4 * sizeof(float);

    ui32 *jointsBuffer   = malloc(jointsBufferSize);
    float *weightsBuffer = malloc(weightsBufferSize);

    // fill joint and weight buffers
    int offset = 0;
    for (int i = 0; i < 1794; i++) {
        cgltf_bool jointsBufferResult = cgltf_accessor_read_uint(
          &data->accessors[3], i, jointsBuffer + offset, 4);

        cgltf_bool weightsBufferResult = cgltf_accessor_read_float(
          &data->accessors[4], i, weightsBuffer + offset, 8);

        if (!jointsBufferResult || !weightsBufferResult) {
            LOG_ERROR("Failed to read skinning data!");
        }
        // step for the next buffer position
        offset += 4;
    }

    // combine joint and weight buffers
    void *skinningBuffer = malloc(jointsBufferSize + weightsBufferSize);
    memcpy(skinningBuffer, jointsBuffer, jointsBufferSize);
    memcpy(((byte *)skinningBuffer)+(jointsBufferSize), weightsBuffer, weightsBufferSize);

    // If we have a skinned mesh
    if (data->skins_count >= 1) {
        Skeleton *skeleton    = malloc(sizeof(Skeleton));
        skeleton->jointsCount = data->skins->joints_count;

        skeleton->joints = malloc(sizeof(Joint *) * data->skins->joints_count);
        for (int i = 0; i < skeleton->jointsCount; i++) {
            skeleton->joints[i] = malloc(sizeof(Joint));
        }

        // Loop through every joint
        for (int i = 0; i < skeleton->jointsCount; i++) {

            Joint joint;

            joint.id            = i;
            joint.name          = strdup(data->skins->joints[i]->name);
            joint.childrenCount = data->skins->joints[i]->children_count;

            // joint.parent = data->skins->joints[i];
            *skeleton->joints[i] = joint;

            for (int j = 0; j < joint.childrenCount; j++) {
                // skeleton->joints[j]->parent = skeleton->joints[i];
            }

            // joint.jointMatrix   = (float *)data->skins->joints[i]->matrix;
        }

        ret->skeleton                    = skeleton;

        ret->skeleton->skinningBuffer    = skinningBuffer;
        ret->skeleton->skinningBufferSize =
          jointsBufferSize + weightsBufferSize;

        ret->skeleton->bufferJoints = jointsBuffer;
        ret->skeleton->bufferJointsSize = jointsBufferSize;

        ret->skeleton->bufferWeights = weightsBuffer;
        ret->skeleton->bufferWeightsSize = weightsBufferSize;

        ret->skeleton->bufferJointsCount = 1794;
    }
    // LOG_INFO("Skeleton ID for bone 0 is %d, name %s, parentId = %d",
    // ret->skeleton->joints[0]->id, ret->skeleton->joints[0]->name,
    // ret->skeleton->joints[1]->parent->id);

    // Cleanup
    cgltf_free(data);
    return ret;
}
