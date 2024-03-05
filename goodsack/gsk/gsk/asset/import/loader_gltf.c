/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "loader_gltf.h"
#pragma optimize("", off)

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/maths.h"

#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/mesh_helpers.inl"

#include "core/graphics/material/material.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define IMPORT_MATERIALS         0
#define STARTING_ANIMATION_INDEX 0

struct AttributeInfo
{
    s32 idxPos;
    s32 idxTex;
    s32 idxNrm;
    s32 idxTan;

    s32 idxJnt;
    s32 idxWht;

    s32 attribCount;

    cgltf_accessor *posData;
    cgltf_accessor *texData;
    cgltf_accessor *nrmData;
    cgltf_accessor *tanData;

    cgltf_accessor *indicesData;
};

static struct AttributeInfo
_get_primitive_attributes(cgltf_primitive *gltfPrimitive)
{

    int attribCount = gltfPrimitive->attributes_count;

    struct AttributeInfo attribInfo = {-1};
    attribInfo.idxTan               = -1;

    for (int i = 0; i < attribCount; i++) {
        const cgltf_attribute *attrib = &gltfPrimitive->attributes[i];

        switch (attrib->type) {
        case cgltf_attribute_type_position:
            attribInfo.idxPos  = i;
            attribInfo.posData = attrib->data;
            break;
        case cgltf_attribute_type_normal:
            attribInfo.idxNrm  = i;
            attribInfo.nrmData = attrib->data;
            break;
        case cgltf_attribute_type_tangent:
            attribInfo.idxTan  = i;
            attribInfo.tanData = attrib->data;
            break;
        case cgltf_attribute_type_texcoord:
            attribInfo.idxTex  = i;
            attribInfo.texData = attrib->data;
            break;
        case cgltf_attribute_type_color: break;
        case cgltf_attribute_type_joints: attribInfo.idxJnt = i; break;
        case cgltf_attribute_type_weights: attribInfo.idxWht = i; break;
        case cgltf_attribute_type_invalid: break;
        default: break;
        }
    }
    attribInfo.attribCount = attribCount;
    attribInfo.indicesData = gltfPrimitive->indices;
    return attribInfo;
}

// Animation Data

static gsk_Animation *
__fill_animation_data(cgltf_animation *gltfAnimation, gsk_Skeleton *skeleton)
{
    u32 inputsCount          = gltfAnimation->samplers[0].input->count;
    gsk_Keyframe **keyframes = malloc(sizeof(gsk_Keyframe *) * inputsCount);

    // Get all frame-times
    float *frameTimes = malloc(inputsCount * sizeof(float));
    for (int i = 0; i < inputsCount; i++) {
        cgltf_bool frameTimesSuccess = cgltf_accessor_read_float(
          gltfAnimation->samplers[0].input, i, frameTimes + i, 8);

        // set keyframe information
        keyframes[i]            = malloc(sizeof(gsk_Keyframe));
        keyframes[i]->frameTime = frameTimes[i];
        keyframes[i]->index     = i;

        keyframes[i]->poses =
          malloc(skeleton->jointsCount * sizeof(gsk_Pose *));

        for (int j = 0; j < skeleton->jointsCount; j++) {
            keyframes[i]->poses[j]            = malloc(sizeof(gsk_Pose));
            keyframes[i]->poses[j]->hasMatrix = 0;
        }
    }

    LOG_INFO("Animation duration: %f\nTotal Keyframes:%d",
             frameTimes[inputsCount - 1],
             inputsCount);

    for (int i = 0; i < gltfAnimation->channels_count; i++) {
        u32 boneIndex = -1;
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

    // Animation data
    gsk_Animation *animation  = malloc(sizeof(gsk_Animation));
    animation->duration       = frameTimes[inputsCount - 1];
    animation->keyframes      = keyframes;
    animation->keyframesCount = inputsCount;
    animation->pSkeleton      = skeleton;
    animation->name           = strdup(gltfAnimation->name);
    animation->index =
      skeleton->animations_count; // current count as opposed to full count

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

static gsk_Joint
_create_joint_recurse(gsk_Skeleton *skeleton,
                      u32 id,
                      gsk_Joint *parent,
                      cgltf_node **jointsNode,
                      cgltf_skin *skinNode)
{
    gsk_Joint joint;
    joint.id             = id;
    joint.name           = jointsNode[id]->name;
    joint.parent         = parent;
    joint.childrenCount  = jointsNode[id]->children_count;
    joint.pose.hasMatrix = 0;

    // inverse bind pose matrix
    mat4 inverseBindPose = GLM_MAT4_ZERO_INIT;
    cgltf_accessor_read_float(skinNode->inverse_bind_matrices,
                              id,
                              (float *)inverseBindPose,
                              32 * sizeof(float));
    glm_mat4_copy(inverseBindPose, joint.mInvBindPose);

    // transformation matrix
    glm_vec3_copy(jointsNode[id]->scale, joint.pose.scale);
    glm_vec4_copy(jointsNode[id]->rotation, joint.pose.rotation);
    glm_vec3_copy(jointsNode[id]->translation, joint.pose.translation);

    mat4 out_matrix = GLM_MAT4_ZERO_INIT;
    // cgltf_node_transform_world(jointsNode[id], (float *)out_matrix);
    _joint_transform_world(&joint, (float *)out_matrix);
    joint.pose.hasMatrix = 0;

    mat4 matrixLocal = GLM_MAT4_ZERO_INIT;
    mat4 init        = GLM_MAT4_IDENTITY_INIT;
    // if (id == 0) glm_mat4_mul(init, out_matrix, out_matrix);
    // if (id == 0) glm_mat4_copy(init, out_matrix);

    glm_mat4_copy(out_matrix, matrixLocal);
    glm_mat4_copy(matrixLocal, joint.pose.mTransform);

    // Allocate and assign
    skeleton->joints[id]  = malloc(sizeof(gsk_Joint));
    *skeleton->joints[id] = joint;

    skeleton->jointsCount = id + 1;

    // Recursive-descent
    for (int i = 0; i < joint.childrenCount; i++) {
        _create_joint_recurse(skeleton,
                              skeleton->jointsCount,
                              skeleton->joints[id],
                              jointsNode,
                              skinNode);
    }

    return joint;
}

// Vertex Data

static gsk_MeshData *
_load_mesh_vertex_data(cgltf_primitive *gltfPrimitive, cgltf_data *data)
{
    gsk_MeshData *ret = malloc(sizeof(gsk_MeshData));

    // TODO: Get more than just the first primitive
    struct AttributeInfo attribInfo = _get_primitive_attributes(gltfPrimitive);

    u32 vertCount      = attribInfo.posData->count;
    ret->vertexCount   = vertCount;
    u32 vPosBufferSize = vertCount * sizeof(float) * 3;
    u32 vTexBufferSize = vertCount * sizeof(float) * 2;
    u32 vNrmBufferSize = vertCount * sizeof(float) * 3;
    u32 vTanBufferSize = vertCount * sizeof(float) * 3 * 2;

    // Required
    ret->buffers.outI = vPosBufferSize + vTexBufferSize + vNrmBufferSize;
    ret->buffers.outI += vTanBufferSize;
    // Add space for tangent data
    if (attribInfo.idxTan > -1) {

        ret->hasTBN = MESH_TBN_MODE_GLTF;
    } else {
        ret->hasTBN = MESH_TBN_MODE_NONE;
        LOG_WARN("Mesh does not contain tangent data");
    }

    // Set min-max bounds
    glm_vec3_copy(attribInfo.posData->min, ret->boundingBox[0]);
    glm_vec3_copy(attribInfo.posData->max, ret->boundingBox[1]);

    ret->buffers.out = malloc(ret->buffers.outI);

    // Position, TextureCoord, Normal

    int offsetA = 0;
    for (int i = 0; i < vertCount; i++) {
        // Fill Positions
        cgltf_accessor_read_float(
          attribInfo.posData, i, ret->buffers.out + offsetA, 100);
        offsetA += 3;
        // Fill TextureCoords
        cgltf_accessor_read_float(
          attribInfo.texData, i, ret->buffers.out + offsetA, 100);
        offsetA += 2;
        // Fill Normals
        cgltf_accessor_read_float(
          attribInfo.nrmData, i, ret->buffers.out + offsetA, 100);
        offsetA += 3;
        // Fill Tangent
        if (ret->hasTBN) {
            cgltf_accessor_read_float(
              attribInfo.tanData, i, ret->buffers.out + offsetA, 100);
            offsetA += 3;
        } else {
            // TODO: calculate TBN
            vec3 vec = GLM_VEC3_ONE_INIT;
            memcpy(ret->buffers.out + offsetA, vec, sizeof(vec3));
            offsetA += 3;
        }
    }
    // TODO: this is kind of goofy.
    ret->hasTBN = MESH_TBN_MODE_GLTF;

    // set this so we push the position to buffer
    ret->buffers.vL  = vertCount;
    ret->buffers.vtL = vertCount;
    ret->buffers.vnL = vertCount;
    // ret->buffers.vnL = 0;

    // Indices //

    ret->indicesCount               = attribInfo.indicesData->count;
    ret->buffers.bufferIndices_size = ret->indicesCount * sizeof(u32);

    ret->buffers.bufferIndices = malloc(ret->buffers.bufferIndices_size);

    // store all indices
    for (int i = 0; i < ret->indicesCount; i++) {
        if (!cgltf_accessor_read_uint(attribInfo.indicesData,
                                      i,
                                      ret->buffers.bufferIndices + i,
                                      ret->indicesCount)) {
            LOG_ERROR("Failed to read uint! %d", i);
        }
    }

    // Skinned mesh

    ret->isSkinnedMesh = data->skins_count;

    // If we have a skinned mesh
    if (ret->isSkinnedMesh >= 1) {

        gsk_Skeleton *skeleton = malloc(sizeof(gsk_Skeleton));
        ret->skeleton          = skeleton;

        // Skeleton information //

        skeleton->jointsCount = data->skins->joints_count;

        cgltf_node *armatureNode = data->scenes[0].nodes[0];

        // Skeleton name from node
        skeleton->name = strdup(armatureNode->name);
        LOG_INFO("Skeleton name: %s\nSkeleton children: %d",
                 skeleton->name,
                 armatureNode->children_count);

        // List of all joints in skeleton
        skeleton->joints =
          malloc(sizeof(gsk_Joint *) * data->skins->joints_count);

        // Create skeleton recursively
        _create_joint_recurse(
          skeleton, 0, NULL, data->skins->joints, &data->skins[0]);
        //_create_joint_recurse(skeleton, 0, NULL, &armatureNode->children[1]);

        // Skinning information //

        u32 jointsBufferSize  = vertCount * 4 * sizeof(u32);
        u32 weightsBufferSize = vertCount * 4 * sizeof(float);

        u32 *jointsBuffer    = malloc(jointsBufferSize);
        float *weightsBuffer = malloc(weightsBufferSize);

        // ret->skeleton->skinningBuffer    = skinningBuffer;
        // ret->skeleton->skinningBufferSize =
        //  jointsBufferSize + weightsBufferSize;

        // fill joint and weight buffers
        int offset = 0;
        for (int i = 0; i < vertCount; i++) {
            cgltf_bool jointsBufferResult = cgltf_accessor_read_uint(
              &data->accessors[attribInfo.idxJnt], i, jointsBuffer + offset, 4);

            cgltf_bool weightsBufferResult =
              cgltf_accessor_read_float(&data->accessors[attribInfo.idxWht],
                                        i,
                                        weightsBuffer + offset,
                                        8);

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

        // Allocate animations
        skeleton->animations_count = animationsCount;
        skeleton->p_animations =
          malloc(sizeof(gsk_Skeleton *) * animationsCount);
        skeleton->cnt_animation_index = STARTING_ANIMATION_INDEX;
        skeleton->animations_count = 0; // NOTE: incremented in loop for reasons

        for (int i = 0; i < animationsCount; i++) {
            LOG_INFO(
              "Animation: \"%s\"\nSamplers count: %d\nChannels count: %d",
              gltfAnimations[i].name,
              gltfAnimations[i].samplers_count,
              gltfAnimations[i].channels_count);

            gsk_Animation *animation =
              __fill_animation_data(&gltfAnimations[i], skeleton);
            skeleton->p_animations[i] = animation;

            // increment so that we can pass the animation index to the actual
            // animation data as well.
            skeleton->animations_count++;
        }

        if (skeleton->animations_count != animationsCount) {
            LOG_ERROR("uh oh");
        }

        // set current animation to the first one in the list
        skeleton->animation =
          skeleton->p_animations[skeleton->cnt_animation_index];
    }
    return ret;
}

// Material Data //

static gsk_Texture *_test_texture_white;
static gsk_Texture *_test_texture_normal;
static gsk_ShaderProgram *s_pbrShader;

static gsk_Texture **s_loaded_textures;
static int s_loaded_textures_count;

#define TEXTURE_POOL_COUNT 70
#define TEST_PATH_URI      "data://textures/sponza/"

gsk_Texture *
__texture_lookup(const char *path, TextureOptions options)
{

    if (s_loaded_textures_count == 0) {
        s_loaded_textures = malloc(sizeof(gsk_Texture *) * TEXTURE_POOL_COUNT);
        s_loaded_textures_count = 1;

        s_loaded_textures[0] = texture_create(strdup(path), NULL, options);
        return s_loaded_textures[0];
    }

    for (int i = 0; i < s_loaded_textures_count; i++) {
        if (!strcmp(s_loaded_textures[i]->filePath, path)) {
            return s_loaded_textures[i];
        }
    }

    // Resize pool if needed
    if (s_loaded_textures_count >= TEXTURE_POOL_COUNT) {
        s_loaded_textures =
          realloc(s_loaded_textures,
                  sizeof(gsk_Texture *) *
                    (s_loaded_textures_count + TEXTURE_POOL_COUNT));
    }

    s_loaded_textures[s_loaded_textures_count] =
      texture_create(strdup(path), NULL, options);
    s_loaded_textures_count += 1;
    return s_loaded_textures[s_loaded_textures_count - 1];
}

static gsk_Material *
_create_material(cgltf_material *gltfMaterial,
                 gsk_Material **materials,
                 u32 materialsCount)
{

    TextureOptions texNormalMapOptions =
      (TextureOptions) {1, GL_RGB, true, false};
    TextureOptions texPbrOptions =
      (TextureOptions) {16, GL_SRGB_ALPHA, true, false};

    // TODO: check if material already exists

    // PBR textures
    if (gltfMaterial->has_pbr_metallic_roughness) {

        gsk_Material *material = gsk_material_create(s_pbrShader, NULL, 0);

        cgltf_pbr_metallic_roughness *textureContainer =
          &gltfMaterial->pbr_metallic_roughness;

        // Base texture
        if (textureContainer->base_color_texture.texture) {
            char p[256] = TEST_PATH_URI;
            const char *diffuseUri =
              textureContainer->base_color_texture.texture->image->uri;
            strcat(p, diffuseUri);
            gsk_material_add_texture(
              material, __texture_lookup(GSK_PATH(p), texPbrOptions));
        } else {
            gsk_material_add_texture(material, _test_texture_white);
        }

        if (gltfMaterial->normal_texture.texture) {

            // Normal texture
            char q[256] = TEST_PATH_URI;
            const char *nrmUri =
              gltfMaterial->normal_texture.texture->image->uri;
            strcat(q, nrmUri);
            gsk_material_add_texture(
              material, __texture_lookup(GSK_PATH(q), texNormalMapOptions));
        } else {
            gsk_material_add_texture(material, _test_texture_normal);
        }

        if (textureContainer->metallic_roughness_texture.texture) {
            // Roughness
            char r[256] = TEST_PATH_URI;
            const char *roughnessUri =
              textureContainer->metallic_roughness_texture.texture->image->uri;
            strcat(r, roughnessUri);
            gsk_material_add_texture(
              material, __texture_lookup(GSK_PATH(r), texPbrOptions));
        } else {
            gsk_material_add_texture(material, _test_texture_white);
        }

        // Specular & AO -- TODO
        gsk_material_add_texture(material, _test_texture_white);
        gsk_material_add_texture(material, _test_texture_white);
        return material;
    }

    // Failed
    return gsk_material_create(
      NULL, GSK_PATH("gsk://shaders/basic_unlit.shader"), 0);
}

// Loader entry //

gsk_Model *
gsk_load_gltf(const char *path, int scale, int importMaterials)
{
    cgltf_options options = {0};
    cgltf_data *data      = NULL;

    cgltf_result result = cgltf_parse_file(&options, path, &data);
    result              = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) {
        LOG_ERROR("Failed to load GLTF file");
    }

    int indicesBufferViewIndex = 0;
    LOG_INFO("Meshes Count: %d", data->meshes_count);

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

        int accessorsCount = data->accessors_count;
    }

    // NOTE: for every single mesh -> go through every pirmitive
    // - primitive is another mesh, ONLY difference is that
    // the model matrix parent is the mesh world-space, not model world-space

    // Figure out how many total objects (meshes) we have
    u32 totalObjects = 0;
    for (int i = 0; i < data->meshes_count; i++) {
        totalObjects += data->meshes[i].primitives_count;
    }

    gsk_Model *ret = malloc(sizeof(gsk_Model));
    ret->meshes    = malloc(sizeof(gsk_Mesh *) * totalObjects);

    // Create texture/material pools
    u32 materialsCount = data->materials_count;
    gsk_Material **materialsPool =
      malloc(sizeof(gsk_Material *) * materialsCount);
    if (importMaterials) {
        _test_texture_white =
          texture_create(GSK_PATH("gsk://textures/defaults/white.png"),
                         NULL,
                         (TextureOptions) {0, GL_RGB, false, false});
        _test_texture_normal =
          texture_create(GSK_PATH("gsk://textures/defaults/normal.png"),
                         NULL,
                         (TextureOptions) {0, GL_RGB, false, false});
        s_pbrShader =
          gsk_shader_program_create(GSK_PATH("gsk://shaders/pbr.shader"));
        s_loaded_textures_count = 0;
    }

    u32 cntMesh = 0;
    for (int i = 0; i < data->nodes_count; i++) {
        // if this node is a Mesh Node
        if (data->nodes[i].mesh != 0) {
            // Each primitive in the mesh
            for (int j = 0; j < data->nodes[i].mesh->primitives_count; j++) {
                gsk_MeshData *meshData = _load_mesh_vertex_data(
                  &data->nodes[i].mesh->primitives[j], data);
                ret->meshes[cntMesh] = gsk_mesh_assemble(meshData);

                mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
                glm_translate(localMatrix, data->nodes[i].translation);
                glm_mat4_copy(localMatrix, ret->meshes[cntMesh]->localMatrix);

                // Add textures to material pools
                if (importMaterials) {

                    // Check for material
                    cgltf_material *gltfMaterial =
                      data->nodes[i].mesh->primitives[j].material;
                    if (gltfMaterial != NULL || gltfMaterial != 0x00) {
                        gsk_Material *mat = _create_material(
                          gltfMaterial, materialsPool, materialsCount);

                        ret->meshes[cntMesh]->materialImported      = mat;
                        ret->meshes[cntMesh]->usingImportedMaterial = TRUE;
                    }
                } else {
                    ret->meshes[cntMesh]->usingImportedMaterial = FALSE;
                }

                cntMesh++;
            }
        }
    }

    // TODO: Move check for skins HERE - OUT of _load_mesh_vertex_data

    ret->modelPath   = path;
    ret->meshesCount = totalObjects;

    LOG_INFO("Loaded textures: %d", s_loaded_textures_count);

    // Cleanup
    cgltf_free(data);
    return ret;
}
