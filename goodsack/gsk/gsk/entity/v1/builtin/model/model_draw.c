/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "model_draw.h"

#include "asset/import/loader_obj.h"
#include "entity/v1/builtin/model/model.h"
#include "entity/v1/builtin/transform/transform.h"
#include "entity/v1/ecs.h"

#include "core/device/device.h"
#include "core/graphics/mesh/mesh.h"
#include "core/graphics/shader/shader.h"

#include "tools/debug/debug_draw_bounds.h"
#include "tools/debug/debug_draw_skeleton.h"

#define DEBUG_DRAW_SKELETON  0
#define DEBUG_DRAW_BOUNDS    0
#define CULLING_FOR_IMPORTED 1

static void
SetCulling(int bitfield)
{
    LOG_INFO("%d", bitfield << 2);
    LOG_INFO("%d", bitfield << 3);
}

static void
DrawModel(struct ComponentModel *model,
          struct ComponentTransform *transform,
          u16 useOverrideMaterial, // Material from renderer
          u32 renderLayer,
          VkCommandBuffer commandBuffer,
          gsk_Renderer *renderer)
{
    if (DEVICE_API_OPENGL) {

// Handle culling
#if 0
         if((mesh->properties.cullMode | CULL_DISABLED)) {
            printf("Disable culling");
        }
#endif
        gsk_Model *pModel = model->pModel;
        for (int i = 0; i < pModel->meshesCount; i++) {
            gsk_Mesh *mesh = pModel->meshes[i];
            gsk_Material *material;

            // Select Material
            if (mesh->usingImportedMaterial && !useOverrideMaterial) {
                material = mesh->materialImported;

#if CULLING_FOR_IMPORTED
                // TODO: temporary solution for face culling.
                // This should be handled by a material property.
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                glFrontFace(GL_CW);
#endif

            } else if (useOverrideMaterial) {
                material = renderer->explicitMaterial;
            } else {
                material = model->material;
            }

            gsk_material_use(material);

            // TESTING for normal-map in G-Buffer
            // TODO: Breaks when normal-map doesn't exist
            if (renderer->currentPass == DEPTH_PREPASS) {
                if (mesh->usingImportedMaterial &&
                    mesh->materialImported->texturesCount > 1) {
                    glActiveTexture(GL_TEXTURE10);
                    texture_bind(mesh->materialImported->textures[1], 10);
                } else if (!mesh->usingImportedMaterial) {
                    glActiveTexture(GL_TEXTURE10);
                    texture_bind(((gsk_Material *)model->material)->textures[1],
                                 10);
                }
            }

            // Skinned Matrix array buffer
            if (mesh->meshData->isSkinnedMesh) {
                mat4 skinnedMatrices[MAX_BONES];
                gsk_Skeleton *pSkeleton = mesh->meshData->skeleton;
                for (int i = 0; i < pSkeleton->jointsCount; i++) {
                    glm_mat4_copy(pSkeleton->joints[i]->pose.mSkinningMatrix,
                                  skinnedMatrices[i]);
                }

                glUniformMatrix4fv(
                  glGetUniformLocation(material->shaderProgram->id,
                                       "u_SkinnedMatrices"),
                  pSkeleton->jointsCount,
                  GL_FALSE,
                  (float *)*skinnedMatrices);
            }
            mat4 newTranslation = GLM_MAT4_ZERO_INIT;
            glm_mat4_mul(mesh->localMatrix, transform->model, newTranslation);
            // Transform Uniform
            glUniformMatrix4fv(
              glGetUniformLocation(material->shaderProgram->id, "u_Model"),
              1,
              GL_FALSE,
              (float *)newTranslation);

            // Light Space Matrix
            glUniformMatrix4fv(glGetUniformLocation(material->shaderProgram->id,
                                                    "u_LightSpaceMatrix"),
                               1,
                               GL_FALSE,
                               (float *)renderer->lightSpaceMatrix);

            // Shadow options
            glUniform1i(
              glGetUniformLocation(material->shaderProgram->id, "u_pcfSamples"),
              renderer->shadowmapOptions.pcfSamples);
            glUniform1f(glGetUniformLocation(material->shaderProgram->id,
                                             "u_normalBiasMin"),
                        renderer->shadowmapOptions.normalBiasMin);
            glUniform1f(glGetUniformLocation(material->shaderProgram->id,
                                             "u_normalBiasMax"),
                        renderer->shadowmapOptions.normalBiasMax);

            // SSAO Options
            glUniform1f(glGetUniformLocation(material->shaderProgram->id,
                                             "u_ssao_strength"),
                        renderer->ssaoOptions.strength);

            // Light strength
            glUniform1f(glGetUniformLocation(material->shaderProgram->id,
                                             "u_light_strength"),
                        renderer->light->strength);

            // Ambient options
            glUniform3fv(
              glGetUniformLocation(material->shaderProgram->id,
                                   "u_ambient_color_multiplier"),
              1,
              (float *)renderer->lightOptions.ambient_color_multiplier);

            glUniform1f(glGetUniformLocation(material->shaderProgram->id,
                                             "u_ambient_strength"),
                        renderer->lightOptions.ambient_strength);
            glUniform1f(glGetUniformLocation(material->shaderProgram->id,
                                             "u_prefilter_strength"),
                        renderer->lightOptions.prefilter_strength);

            // Set the correct camera layer
            glUniform1i(glGetUniformLocation(material->shaderProgram->id,
                                             "u_render_layer"),
                        renderLayer);

            vao_bind(mesh->vao);

            gsk_MeshData *data = mesh->meshData;
            u32 vertices  = data->vertexCount;
            u32 indices   = data->indicesCount;

            u16 drawMode = model->properties.drawMode;

            // glEnable(GL_CULL_FACE);
            // glCullFace(GL_BACK);
            // glFrontFace(GL_CW);

            switch (drawMode) {
            case DRAW_ARRAYS: glDrawArrays(GL_TRIANGLES, 0, vertices); break;
            case DRAW_ELEMENTS:
                glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, NULL);
                break;
            case DRAW_ELEMENTS_WIREFRAME:
                glDrawElements(GL_LINES, indices, GL_UNSIGNED_INT, NULL);
            }
        }
        glDisable(GL_CULL_FACE);

    } else if (DEVICE_API_VULKAN) {

#if 0
        // Bind transform Model (MVP) and Position (vec3) descriptor set
        vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                context->pipelineDetails->pipelineLayout, 0, 1,
                &context->descriptorSets[context->currentFrame], 0, NULL);
        // Bind texture descriptor set
        vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                context->pipelineDetails->pipelineLayout, 0, 1,
                &context->descriptorSets[context->currentFrame], 0, NULL);
        // Bind Vertex/Index buffers
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        // Draw command
        vkCmdDraw(commandBuffer, context->vertexBuffer->size, 1, 0, 0);
#endif
        // Bind Vertex/Index buffers
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(
          commandBuffer, 0, 1, &((gsk_Mesh *)model->mesh)->vkVBO->buffer, offsets);

        // Draw command
        vkCmdDraw(commandBuffer, ((gsk_Mesh *)model->mesh)->vkVBO->size, 1, 0, 0);
    }
}

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;
    if (!(gsk_ecs_has(e, C_MODEL))) return;

    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
    struct ComponentModel *model         = gsk_ecs_get(e, C_MODEL);

    // TODO: stupid hack grabbing only scale.x...
    // mesh->model = gsk_load_obj(mesh->modelPath, transform->scale[0]);

    if (DEVICE_API_OPENGL) {
        if (!(gsk_Model *)model->pModel) {
            model->pModel = gsk_model_load_from_file(
              model->modelPath, transform->scale[0], FALSE);
        }
        model->mesh = ((gsk_Model *)model->pModel)->meshes[0];
        // send lightspace matrix from renderer to entity shader
        gsk_ShaderProgram *shader = ((gsk_Material *)model->material)->shaderProgram;
        gsk_shader_use(shader);
        glUniformMatrix4fv(
          glGetUniformLocation(shader->id, "u_LightSpaceMatrix"),
          1,
          GL_FALSE,
          (float *)e.ecs->renderer->lightSpaceMatrix);
        // TODO: send model matrix to shader
    } else if (DEVICE_API_VULKAN) {
        model->mesh = malloc(sizeof(gsk_Mesh));
        ((gsk_Mesh *)(model->mesh))->meshData =
          gsk_load_obj(model->modelPath, transform->scale[0]);

        VulkanDeviceContext *context = e.ecs->renderer->vulkanDevice;

        model->vkVBO = vulkan_vertex_buffer_create(
          context->physicalDevice,
          context->device,
          context->graphicsQueue,
          context->commandPool,
          ((gsk_Mesh *)model->mesh)->meshData->buffers.out,
          ((gsk_Mesh *)model->mesh)->meshData->buffers.outI * sizeof(float));
    }
}

static void
render(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;
    if (!(gsk_ecs_has(e, C_MODEL))) return;
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
    struct ComponentModel *model         = gsk_ecs_get(e, C_MODEL);

    u32 renderLayer = 0; // DEFAULT RENDER LAYER when not specified.
    if (gsk_ecs_has(e, C_RENDERLAYER)) {
        renderLayer =
          (u32)((struct ComponentRenderLayer *)gsk_ecs_get(e, C_RENDERLAYER))
            ->renderLayer;
    }

    RenderPass pass = e.ecs->renderer->currentPass;

    // TODO: get lightspace matrix

    VkCommandBuffer cb;
    if (DEVICE_API_VULKAN) {
        cb = e.ecs->renderer->vulkanDevice
               ->commandBuffers[e.ecs->renderer->vulkanDevice->currentFrame];
    }

    if (pass == REGULAR) {
#if DEBUG_DRAW_SKELETON
        // draw skeleton
        if (model->mesh->meshData->isSkinnedMesh) {
            gsk_debug_draw_skeleton(e.ecs->renderer->debugContext,
                                model->mesh->meshData->skeleton);
        }
#endif

#if DEBUG_DRAW_BOUNDS

        gsk_Model *pModel = model->pModel;
        for (int i = 0; i < pModel->meshesCount; i++) {
            Mesh *mesh = pModel->meshes[i];
            gsk_debug_draw_bounds(e.ecs->renderer->debugContext,
                              mesh->meshData->boundingBox,
                              transform->model);
        }
#endif

        // Regular Render
        (DEVICE_API_OPENGL)
          ? DrawModel(
              model, transform, FALSE, renderLayer, NULL, e.ecs->renderer)
          : DrawModel(
              model, transform, FALSE, renderLayer, cb, e.ecs->renderer);

    } else {
        (DEVICE_API_OPENGL)
          ? DrawModel(
              model, transform, TRUE, renderLayer, NULL, e.ecs->renderer)
          : DrawModel(model, transform, TRUE, renderLayer, cb, e.ecs->renderer);
    }
}

void
s_model_draw_init(gsk_ECS *ecs)
{
    //_ECS_DECL_COMPONENT(ecs, C_MODEL, sizeof(struct ComponentModel));
    gsk_ecs_system_register(ecs,
                        ((gsk_ECSSystem) {
                          .init    = (gsk_ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = (gsk_ECSSubscriber)render,
                          .update  = NULL,
                        }));
}
