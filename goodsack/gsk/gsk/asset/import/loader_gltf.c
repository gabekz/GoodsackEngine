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

#define IMPORT_MATERIALS 0

struct AttributeInfo
{
    si32 idxPos;
    si32 idxTex;
    si32 idxNrm;
    si32 idxTan;

    si32 idxJnt;
    si32 idxWht;

    si32 attribCount;

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

static Animation *
__fill_animation_data(cgltf_animation *gltfAnimation, Skeleton *skeleton)
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

    // TODO: set correct iterator
    // for (int i = 0; i < gltfAnimation->channels_count; i++) {
    for (int i = 0; i < (skeleton->jointsCount * 3); i++) {
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

    // Animation data
    Animation *animation      = malloc(sizeof(Animation));
    animation->duration       = frameTimes[inputsCount - 1];
    animation->keyframes      = keyframes;
    animation->keyframesCount = inputsCount;
    animation->pSkeleton      = skeleton;

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

static Joint
_create_joint_recurse(Skeleton *skeleton,
                      ui32 id,
                      Joint *parent,
                      cgltf_node **jointsNode,
                      cgltf_skin *skinNode)
{
    Joint joint;
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
    skeleton->joints[id]  = malloc(sizeof(Joint));
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

static MeshData *
_load_mesh_vertex_data(cgltf_primitive *gltfPrimitive, cgltf_data *data)
{
    MeshData *ret = malloc(sizeof(MeshData));

    // TODO: Get more than just the first primitive
    struct AttributeInfo attribInfo = _get_primitive_attributes(gltfPrimitive);

    ui32 vertCount      = attribInfo.posData->count;
    ret->vertexCount    = vertCount;
    ui32 vPosBufferSize = vertCount * sizeof(float) * 3;
    ui32 vTexBufferSize = vertCount * sizeof(float) * 2;
    ui32 vNrmBufferSize = vertCount * sizeof(float) * 3;
    ui32 vTanBufferSize = vertCount * sizeof(float) * 3 * 2;

    // Required
    ret->buffers.outI = vPosBufferSize + vTexBufferSize + vNrmBufferSize;

    // Add space for tangent data
    if (attribInfo.idxTan > -1) {
        ret->buffers.outI += vTanBufferSize;
        ret->hasTBN = 2; // TODO
    } else {
        ret->hasTBN = 0;
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
        if (attribInfo.idxTan > -1) {
            cgltf_accessor_read_float(
              attribInfo.tanData, i, ret->buffers.out + offsetA, 100);
            offsetA += 3;
        }
    }

    // Tangent

    // set this so we push the position to buffer
    ret->buffers.vL  = vertCount;
    ret->buffers.vtL = vertCount;
    ret->buffers.vnL = vertCount;
    // ret->buffers.vnL = 0;

    // Indices //

    ret->indicesCount               = attribInfo.indicesData->count;
    ret->buffers.bufferIndices_size = ret->indicesCount * sizeof(ui32);

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
        _create_joint_recurse(
          skeleton, 0, NULL, data->skins->joints, &data->skins[0]);
        //_create_joint_recurse(skeleton, 0, NULL, &armatureNode->children[1]);

        // Skinning information //

        ui32 jointsBufferSize  = vertCount * 4 * sizeof(ui32);
        ui32 weightsBufferSize = vertCount * 4 * sizeof(float);

        ui32 *jointsBuffer   = malloc(jointsBufferSize);
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

        for (int i = 0; i < animationsCount; i++) {
            LOG_INFO(
              "Animation: \"%s\"\nSamplers count: %d\nChannels count: %d",
              gltfAnimations[i].name,
              gltfAnimations[i].samplers_count,
              gltfAnimations[i].channels_count);

            Animation *animation =
              __fill_animation_data(&gltfAnimations[i], skeleton);
            //_skeleton_set_keyframe(
            //  animation, 0); // sets all the skeleton poses to keyframe 20

            skeleton->animation = animation;
        }
    }
    return ret;
}

// Material Data //

static Texture *_test_texture_white;
static Texture *_test_texture_normal;
static ShaderProgram *s_pbrShader;

static Texture **s_loaded_textures;
static int s_loaded_textures_count;

#define TEXTURE_POOL_COUNT 70
#define TEST_PATH_URI      "data://textures/sponza/"

Texture *
__texture_lookup(const char *path, TextureOptions options)
{

    if (s_loaded_textures_count == 0) {
        s_loaded_textures = malloc(sizeof(Texture *) * TEXTURE_POOL_COUNT);
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
        s_loaded_textures = realloc(
          s_loaded_textures,
          sizeof(Texture *) * (s_loaded_textures_count + TEXTURE_POOL_COUNT));
    }

    s_loaded_textures[s_loaded_textures_count] =
      texture_create(strdup(path), NULL, options);
    s_loaded_textures_count += 1;
    return s_loaded_textures[s_loaded_textures_count - 1];
}

static Material *
_create_material(cgltf_material *gltfMaterial,
                 Material **materials,
                 ui32 materialsCount)
{

    TextureOptions texNormalMapOptions =
      (TextureOptions) {1, GL_RGB, false, false};
    TextureOptions texPbrOptions =
      (TextureOptions) {16, GL_SRGB_ALPHA, true, false};

    // TODO: check if material already exists

    // PBR textures
    if (gltfMaterial->has_pbr_metallic_roughness) {

        Material *material = material_create(s_pbrShader, NULL, 0);

        cgltf_pbr_metallic_roughness *textureContainer =
          &gltfMaterial->pbr_metallic_roughness;

        // Base texture
        if (textureContainer->base_color_texture.texture) {
            char p[256] = TEST_PATH_URI;
            const char *diffuseUri =
              textureContainer->base_color_texture.texture->image->uri;
            strcat(p, diffuseUri);
            material_add_texture(material,
                                 __texture_lookup(GSK_PATH(p), texPbrOptions));
        } else {
            material_add_texture(material, _test_texture_white);
        }

        if (gltfMaterial->normal_texture.texture) {

            // Normal texture
            char q[256] = TEST_PATH_URI;
            const char *nrmUri =
              gltfMaterial->normal_texture.texture->image->uri;
            strcat(q, nrmUri);
            material_add_texture(
              material, __texture_lookup(GSK_PATH(q), texNormalMapOptions));
        } else {
            material_add_texture(material, _test_texture_normal);
        }

        if (textureContainer->metallic_roughness_texture.texture) {
            // Roughness
            char r[256] = TEST_PATH_URI;
            const char *roughnessUri =
              textureContainer->metallic_roughness_texture.texture->image->uri;
            strcat(r, roughnessUri);
            material_add_texture(material,
                                 __texture_lookup(GSK_PATH(r), texPbrOptions));
        } else {
            material_add_texture(material, _test_texture_white);
        }

        // Specular & AO -- TODO
        material_add_texture(material, _test_texture_white);
        material_add_texture(material, _test_texture_white);
        return material;
    }

    // Failed
    return material_create(NULL, GSK_PATH("gsk://shaders/white.shader"), 0);
}

// Loader entry //

Model *
load_gltf(const char *path, int scale, int importMaterials)
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
    ui32 totalObjects = 0;
    for (int i = 0; i < data->meshes_count; i++) {
        totalObjects += data->meshes[i].primitives_count;
    }

    Model *ret  = malloc(sizeof(Model));
    ret->meshes = malloc(sizeof(Mesh *) * totalObjects);

    // Create texture/material pools
    ui32 materialsCount      = data->materials_count;
    Material **materialsPool = malloc(sizeof(Material *) * materialsCount);
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
          shader_create_program(GSK_PATH("gsk://shaders/pbr.shader"));
        s_loaded_textures_count = 0;
    }

    ui32 cntMesh = 0;
    for (int i = 0; i < data->nodes_count; i++) {
        // if this node is a Mesh Node
        if (data->nodes[i].mesh != 0) {
            // Each primitive in the mesh
            for (int j = 0; j < data->nodes[i].mesh->primitives_count; j++) {
                MeshData *meshData = _load_mesh_vertex_data(
                  &data->nodes[i].mesh->primitives[j], data);
                ret->meshes[cntMesh] = mesh_assemble(meshData);

                mat4 localMatrix = GLM_MAT4_IDENTITY_INIT;
                glm_translate(localMatrix, data->nodes[i].translation);
                glm_mat4_copy(localMatrix, ret->meshes[cntMesh]->localMatrix);

                // Add textures to material pools
                if (importMaterials) {

                    // Check for material
                    cgltf_material *gltfMaterial =
                      data->nodes[i].mesh->primitives[j].material;
                    if (gltfMaterial != NULL || gltfMaterial != 0x00) {
                        Material *mat = _create_material(
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
