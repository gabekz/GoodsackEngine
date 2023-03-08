#include "loader_gltf.h"
#pragma optimize("", off)

#include <core/graphics/mesh/mesh.h>
#include <util/logger.h>
#include <util/maths.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

//#define LOGGING_GLTF

void
joint_transform_local(Joint *joint, float *outMatrix)
{
    float *lm = outMatrix;

    if (joint->pose.hasMatrix) {
        memcpy(lm, joint->pose.mTransform, sizeof(float) * 16);
    } else {
        Pose *node = &joint->pose;
        float tx   = node->translation[0];
        float ty   = node->translation[1];
        float tz   = node->translation[2];

        float qx = node->rotation[0];
        float qy = node->rotation[1];
        float qz = node->rotation[2];
        float qw = node->rotation[3];

        float sx = node->scale[0];
        float sy = node->scale[1];
        float sz = node->scale[2];

        lm[0] = (1 - 2 * qy * qy - 2 * qz * qz) * sx;
        lm[1] = (2 * qx * qy + 2 * qz * qw) * sx;
        lm[2] = (2 * qx * qz - 2 * qy * qw) * sx;
        lm[3] = 0.f;

        lm[4] = (2 * qx * qy - 2 * qz * qw) * sy;
        lm[5] = (1 - 2 * qx * qx - 2 * qz * qz) * sy;
        lm[6] = (2 * qy * qz + 2 * qx * qw) * sy;
        lm[7] = 0.f;

        lm[8]  = (2 * qx * qz + 2 * qy * qw) * sz;
        lm[9]  = (2 * qy * qz - 2 * qx * qw) * sz;
        lm[10] = (1 - 2 * qx * qx - 2 * qy * qy) * sz;
        lm[11] = 0.f;

        lm[12] = tx;
        lm[13] = ty;
        lm[14] = tz;
        lm[15] = 1.f;
    }
}

void
joint_transform_world(Joint *joint, float *outMatrix)
{
    float *lm = outMatrix;
    joint_transform_local(joint, lm);

    const Joint *parent = joint->parent;

    while (parent) {
        float pm[16];
        joint_transform_local(parent, pm);

        for (int i = 0; i < 4; ++i) {

            float l0 = lm[i * 4 + 0];
            float l1 = lm[i * 4 + 1];
            float l2 = lm[i * 4 + 2];

            float r0 = l0 * pm[0] + l1 * pm[4] + l2 * pm[8];
            float r1 = l0 * pm[1] + l1 * pm[5] + l2 * pm[9];
            float r2 = l0 * pm[2] + l1 * pm[6] + l2 * pm[10];

            lm[i * 4 + 0] = r0;
            lm[i * 4 + 1] = r1;
            lm[i * 4 + 2] = r2;
        }

        lm[12] += pm[12];
        lm[13] += pm[13];
        lm[14] += pm[14];

        parent = parent->parent;
    }
}

static Animation *
_fill_animation_data(cgltf_animation *gltfAnimation, Skeleton *skeleton)
{
    ui32 inputsCount     = gltfAnimation->samplers[0].input->count;
    Keyframe **keyframes = malloc(sizeof(Keyframe *) * inputsCount);

    // Get all frame-times
    float *frameTimes = malloc(inputsCount * sizeof(float));
    for (int i = 0; i < inputsCount; i++) {
        cgltf_bool frameTimesSuccess = cgltf_accessor_read_float(
          gltfAnimation->samplers[0].input, i, frameTimes + i, 8);

        // set keyframe information
        keyframes[i]            = malloc(sizeof(Keyframe));
        keyframes[i]->frameTime = frameTimes[i];
        keyframes[i]->index     = i;

        keyframes[i]->poses = malloc(skeleton->jointsCount * sizeof(Pose *));

        for (int j = 0; j < skeleton->jointsCount; j++) {
            keyframes[i]->poses[j]            = malloc(sizeof(Pose));
            keyframes[i]->poses[j]->hasMatrix = 0;
        }
    }

    LOG_INFO("Animation duration: %f\nTotal Keyframes:%d",
             frameTimes[inputsCount - 1],
             inputsCount);

    // for (int i = 0; i < gltfAnimation->channels_count; i++) {
    for (int i = 0; i < 45; i++) {
        ui32 boneIndex = -1;
        // Go through each bone and find ID by target_node of channel
        // TODO: very, very slow. Fix this later.
        for (int j = 0; j < skeleton->jointsCount; j++) {
            if (!strncmp(skeleton->joints[j]->name,
                         gltfAnimation->channels[i].target_node->name,
                         MAX_BONE_NAME_LEN)) {
                boneIndex = skeleton->joints[j]->id;
                // TODO: Get parent index by boneIndex
                break;
            }
        }
        if (boneIndex == -1) LOG_ERROR("Failed to find bone index");

        // Set parameters
        switch (gltfAnimation->channels[i].target_path) {
        case cgltf_animation_path_type_translation:

            for (int j = 0; j < inputsCount; j++) {
                vec3 output            = GLM_VEC3_ZERO_INIT;
                cgltf_bool testSuccess = cgltf_accessor_read_float(
                  gltfAnimation->channels[i].sampler->output, j, output, 8);

                glm_vec3_zero(keyframes[j]->poses[boneIndex]->translation);
                glm_vec3_copy(output,
                              keyframes[j]->poses[boneIndex]->translation);
            }
            break;

        case cgltf_animation_path_type_rotation:
            for (int j = 0; j < inputsCount; j++) {
                float *output          = GLM_VEC4_ZERO;
                cgltf_bool testSuccess = cgltf_accessor_read_float(
                  gltfAnimation->channels[i].sampler->output, j, output, 8);

                glm_vec4_copy(output, keyframes[j]->poses[boneIndex]->rotation);
            }
            break;
        case cgltf_animation_path_type_scale:
            for (int j = 0; j < inputsCount; j++) {
                float *output          = GLM_VEC3_ZERO;
                cgltf_bool testSuccess = cgltf_accessor_read_float(
                  gltfAnimation->channels[i].sampler->output, j, output, 8);

                glm_vec3_copy(output, keyframes[j]->poses[boneIndex]->scale);
            }
            break;
        default: break;
        }
    }

    // Loop through every bone again per keyframe, and
    // create a transform matrix for each bone
    for (int i = 0; i < inputsCount; i++) {
        /* for (int j = 0; j < skeleton->jointsCount; j++) {
            // TODO: Get parent index by boneIndex
            skeleton->joints[j]->pose = *keyframes[i]->poses[j];
        }*/
        for (int j = 0; j < skeleton->jointsCount; j++) {
            mat4 output               = GLM_MAT4_ZERO_INIT;
            skeleton->joints[j]->pose = *keyframes[i]->poses[j];
            joint_transform_world(skeleton->joints[j], (float *)output);
            glm_mat4_copy(output, keyframes[i]->poses[j]->mTransform);
        }
    }

#if 0
            Pose newPose             = _keyframePose();
            newPose.translation = anim.samplers[0].output->buffer_view->data[0];
            keyframes->poses[boneId] = newPose;
#endif

    // Animation data
    Animation *animation = malloc(sizeof(Animation));
    animation->duration  = frameTimes[inputsCount - 1];
    animation->keyframes = keyframes;
    animation->keyframesCount = inputsCount;
    animation->pSkeleton = skeleton;

#if 0
#define TEST_BONE 2
    for (int i = 0; i < inputsCount; i++) {
        LOG_INFO("Bone rotation @ keyframe %d for bone %d: %f\t%f\t%f\t%f", 
                i,
                TEST_BONE,
                 animation->keyframes[i]->poses[TEST_BONE]->rotation[0],
                 animation->keyframes[i]->poses[TEST_BONE]->rotation[1],
                 animation->keyframes[i]->poses[TEST_BONE]->rotation[2],
                 animation->keyframes[i]->poses[TEST_BONE]->rotation[3]);
    }
#endif

    return animation;
}

static void
_skeleton_set_keyframe(Animation *animation, ui32 cntKeyframe)
{
    // get keyframe based on: t = delta / prev+next

    Skeleton *skeleton = animation->pSkeleton;
    for (int i = 0; i < skeleton->jointsCount; i++) {

        Pose currentPose = skeleton->joints[i]->pose;
        Pose nextPose    = *animation->keyframes[cntKeyframe]->poses[i];

        // Pose newPose = LERP(currentPose, newPose, t);
        Pose newPose              = nextPose;
        skeleton->joints[i]->pose = nextPose;
    }
}

#if 0
static void
_play_animation(Animation *animation)
{
    ui32 keyframe = 0;
    _update_all_poses(animation, keyframe);
}
#endif

static Joint
_create_joint_recurse(Skeleton *skeleton,
                      ui32 id,
                      Joint *parent,
                      cgltf_node **jointsNode)
{
    Joint joint;
    joint.id             = id;
    joint.name           = jointsNode[id]->name;
    joint.parent         = parent;
    joint.childrenCount  = jointsNode[id]->children_count;
    joint.pose.hasMatrix = 0;

    glm_vec3_copy(jointsNode[id]->scale, joint.pose.scale);
    glm_vec4_copy(jointsNode[id]->rotation, joint.pose.rotation);
    glm_vec3_copy(jointsNode[id]->translation, joint.pose.translation);

    mat4 out_matrix = GLM_MAT4_ZERO_INIT;
    // cgltf_node_transform_world(jointsNode[id], (float *)out_matrix);
    joint_transform_world(&joint, (float *)out_matrix);
    joint.pose.hasMatrix = 0;

    mat4 matrixLocal = GLM_MAT4_ZERO_INIT;
    glm_mat4_copy(out_matrix, matrixLocal);
    glm_mat4_copy(matrixLocal, joint.pose.mTransform);

    // Allocate and assign
    skeleton->joints[id]  = malloc(sizeof(Joint));
    *skeleton->joints[id] = joint;

    skeleton->jointsCount = id + 1;

    // Recursive-descent
    for (int i = 0; i < joint.childrenCount; i++) {
        _create_joint_recurse(
          skeleton, skeleton->jointsCount, skeleton->joints[id], jointsNode);
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

        int accessorsCount = data->accessors_count;
#ifdef LOGGING_GLTF

        LOG_INFO(
          "\nIndices count: %d\tbuffer size: %d", indices->count, indicesSize);

        LOG_INFO("\nAccessors count: %d", accessorsCount);
        for (int i = 0; i < accessorsCount; i++) {
            cgltf_accessor accessor = data->accessors[i];
            LOG_INFO("Accessor: %d\tBufferView size: %d",
                     i,
                     accessor.buffer_view->size);
        }
#endif // LOGGING_GLTF
    }

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
    // void *skinningBuffer = malloc(jointsBufferSize + weightsBufferSize);
    // memcpy(skinningBuffer, jointsBuffer, jointsBufferSize);
    // memcpy(((byte *)skinningBuffer)+(jointsBufferSize), weightsBuffer,
    // weightsBufferSize);

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
        LOG_INFO("Skeleton name: %s\nSkeleton children: %d",
                 skeleton->name,
                 armatureNode->children_count);

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

        // ret->skeleton->skinningBuffer    = skinningBuffer;
        // ret->skeleton->skinningBufferSize =
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
        skeleton->bufferJoints     = jointsBuffer;
        skeleton->bufferJointsSize = jointsBufferSize;

        skeleton->bufferWeights     = weightsBuffer;
        skeleton->bufferWeightsSize = weightsBufferSize;

        // Animations //

        int animationsCount             = data->animations_count;
        cgltf_animation *gltfAnimations = data->animations;

        LOG_INFO("Animations: %d", animationsCount);

        for (int i = 0; i < animationsCount; i++) {
            LOG_INFO(
              "Animation: \"%s\"\nSamplers count: %d\nChannels count: %d",
              gltfAnimations[i].name,
              gltfAnimations[i].samplers_count,
              gltfAnimations[i].channels_count);

            Animation *animation =
              _fill_animation_data(&gltfAnimations[i], skeleton);
            _skeleton_set_keyframe(
              animation, 0); // sets all the skeleton poses to keyframe 20

            skeleton->animation = animation;
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
