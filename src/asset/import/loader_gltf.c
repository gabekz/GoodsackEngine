#include "loader_gltf.h"

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
            LOG_INFO("%d\tAttribute: %s - buffer index: %d", j, attribute.name, attribute.index);
            indicesBufferViewIndex++;
        }

        cgltf_accessor *indices = data->meshes[i].primitives->indices;

        cgltf_size indicesSize =
          data->accessors[indicesBufferViewIndex].count *
          sizeof(
            unsigned short); // cgltf_accessor_read_index(data->accessors, 3);

        //LOG_INFO("\nIndices count: %d\tbuffer size: %d", indices->count, data->accessors[indicesBufferViewIndex].count * sizeof(unsigned short));
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
    cgltf_accessor_unpack_floats(data->accessors,
        ret->buffers.out, 1794 * sizeof(vec3)); // UNPACK for out -> read_float for individual

    // total size of output
    ret->buffers.outI = 1794 * sizeof(float) * sizeof(vec3);

    ret->vertexCount = 1794;
    
    // set this so we push the position to buffer
    ret->buffers.vL = 1794;
    ret->buffers.vtL = 0;
    ret->buffers.vnL = 0;
    //ret->buffers.vnL = 0;

    // Indices //

    ret->indicesCount = data->accessors[indicesBufferViewIndex].count;
    ret->buffers.bufferIndices_size = ret->indicesCount * sizeof(ui32);

    ret->buffers.bufferIndices = malloc(ret->buffers.bufferIndices_size);

    // store all indices
    for (int i = 0; i < ret->indicesCount; i++) {
    if (!cgltf_accessor_read_uint(
        &data->accessors[indicesBufferViewIndex], i, ret->buffers.bufferIndices+i, ret->indicesCount)) {
        LOG_ERROR("Failed to read uint! %d", i);
        }
    }

    // Cleanup
    cgltf_free(data);
    return ret;
}
