#include "loader_gltf.h"
#pragma optimize("", off)

#include <core/graphics/mesh/mesh.h>
#include <util/logger.h>
#include <util/maths.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define LOGGING_GLTF

static Joint
_create_joint_recurse(Skeleton *skeleton, ui32 id, Joint *parent, cgltf_node **jointsNode)
{
    Joint joint;
    joint.id = id;
    joint.name = jointsNode[id]->name;
    joint.parent = parent;
    joint.childrenCount = jointsNode[id]->children_count;

    glm_vec3_copy(jointsNode[id]->scale, joint.scale);
    glm_vec4_copy(jointsNode[id]->rotation, joint.rotation);
    glm_vec3_copy(jointsNode[id]->translation, joint.translation);

    mat4 out_matrix = GLM_MAT4_ZERO_INIT;
    cgltf_node_transform_world(jointsNode[id], (float *)out_matrix);

    mat4 matrixLocal = GLM_MAT4_ZERO_INIT;
    glm_mat4_copy(out_matrix, matrixLocal);

    /*

    // calculate scale matrix
    mat4 matrixScale = GLM_MAT4_IDENTITY_INIT;
    glm_scale(matrixScale, joint.scale);

    // calculate rotation matrix
    mat4 matrixRotation = GLM_MAT4_IDENTITY_INIT;
    glm_translate(matrixRotation, (vec3) {0, 0, 0});
    versor quaternion;
    glm_quat_init(quaternion, joint.rotation[0], joint.rotation[1], joint.rotation[2], joint.rotation[3]);
    cgltf_float *test = jointsNode[id]->matrix;
    LOG_INFO("%f", test[0]);
    //glm_quat_mat4(matrixRotation, quaternion, matrixRotation);
    //glm_quat_mat4(quaternion, matrixRotation);
    glm_quat_mat4t(quaternion, matrixRotation);

    // calculate translation matrix
    mat4 matrixTranslation = GLM_MAT4_IDENTITY_INIT;
    glm_translate(matrixTranslation, joint.translation);
    
    // calculate combined-matrix 
    mat4 matrixLocal = GLM_MAT4_IDENTITY_INIT;
    //glm_mat4_mul(matrixScale, matrixRotation, matrixLocal);
    //glm_mat4_mul(matrixScale, matrixRotation, matrixRotation);
    //glm_mat4_mul(matrixTranslation, matrixRotation, matrixLocal);
    glm_mat4_mul(matrixTranslation, matrixRotation, matrixLocal);
    glm_mat4_mul(matrixLocal, matrixScale, matrixLocal);
    //glm_quat_rotate(matrixTranslation, quaternion, matrixTranslation);

    //glm_mat4_copy(matrixTranslation, matrixLocal);
    */

    // Multiply by hierarchy
    if (parent == NULL || id == 0) {
        glm_mat4_copy(matrixLocal, joint.matrix);
    } else {
        glm_mat4_copy(matrixLocal, joint.matrix);
        //glm_mat4_mul(matrixLocal, joint.parent->matrix, joint.matrix);
    }

    // Allocate and assign
    skeleton->joints[id] = malloc(sizeof(Joint));
    *skeleton->joints[id] = joint;

    skeleton->jointsCount = id + 1;


    // Recursive-descent
    for (int i = 0; i < joint.childrenCount; i++)
    {
        _create_joint_recurse(skeleton, skeleton->jointsCount, skeleton->joints[id], jointsNode);
    }

    return joint;
}

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

    // combine joint and weight buffers
    //void *skinningBuffer = malloc(jointsBufferSize + weightsBufferSize);
    //memcpy(skinningBuffer, jointsBuffer, jointsBufferSize);
    //memcpy(((byte *)skinningBuffer)+(jointsBufferSize), weightsBuffer, weightsBufferSize);

    // If we have a skinned mesh
    if (data->skins_count >= 1) {

        ret->isSkinnedMesh = 1;

        Skeleton *skeleton = malloc(sizeof(Skeleton));
        ret->skeleton      = skeleton;

        // Skeleton information //

        skeleton->jointsCount = data->skins->joints_count;

        cgltf_node *armatureNode = data->scenes[0].nodes[0];

        // Skeleton name from node
        skeleton->name = strdup(armatureNode->name);
        LOG_INFO("Skeleton name: %s\nSkeleton children: %d", skeleton->name, armatureNode->children_count);

        // List of all joints in skeleton
        skeleton->joints = malloc(sizeof(Joint *) * data->skins->joints_count);

        // Create skeleton recursively
        _create_joint_recurse(skeleton, 0, NULL, data->skins->joints);
        //_create_joint_recurse(skeleton, 0, NULL, &armatureNode->children[1]);

        // Skinning information //

        ui32 jointsBufferSize  = 1794 * 4 * sizeof(ui32);
        ui32 weightsBufferSize = 1794 * 4 * sizeof(float);

        ui32 *jointsBuffer   = malloc(jointsBufferSize);
        float *weightsBuffer = malloc(weightsBufferSize);

        //ret->skeleton->skinningBuffer    = skinningBuffer;
        //ret->skeleton->skinningBufferSize =
        //  jointsBufferSize + weightsBufferSize;

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
        skeleton->bufferJoints = jointsBuffer;
        skeleton->bufferJointsSize = jointsBufferSize;

        skeleton->bufferWeights = weightsBuffer;
        skeleton->bufferWeightsSize = weightsBufferSize;

        // Animations //

        int animationsCount          = data->animations_count;
        cgltf_animation *animations = data->animations;

        LOG_INFO("Animations: %d", animationsCount);

        for (int i = 0; i < animationsCount; i++) {
            cgltf_animation anim = animations[i];
            LOG_INFO("Animation: \"%s\"\nSamplers count: %d\nChannels count: %d", anim.name, anim.samplers_count, anim.channels_count);
        }

    } // IF skinnedMesh

    // Testing
#if 0
    LOG_INFO("Skeleton ID for bone 0 is %d, name %s, parentId = %d",
    ret->skeleton->joints[0]->id, ret->skeleton->joints[0]->name,
    ret->skeleton->joints[1]->parent->id);
#endif

    // Cleanup
    cgltf_free(data);
    return ret;
}
