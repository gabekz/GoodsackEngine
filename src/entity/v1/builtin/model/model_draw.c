#include "model_draw.h"

#include <entity/v1/builtin/model/model.h>
#include <entity/v1/builtin/transform/transform.h>

#include <asset/import/loader_obj.h>
#include <core/graphics/mesh/mesh.h>
#include <core/graphics/shader/shader.h>
#include <entity/v1/ecs.h>

#include <core/device/device.h>

#include <tools/debug/debug_draw_bounds.h>
#include <tools/debug/debug_draw_skeleton.h>

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
          ui16 useOverrideMaterial, // Material from renderer
          VkCommandBuffer commandBuffer,
          Renderer *renderer)
{
    if (DEVICE_API_OPENGL) {

// Handle culling
#if 0
         if((mesh->properties.cullMode | CULL_DISABLED)) {
            printf("Disable culling");
        }
#endif
        Model *pModel = model->pModel;
        for (int i = 0; i < pModel->meshesCount; i++) {
            Mesh *mesh = pModel->meshes[i];
            Material *material;

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

            material_use(material);

            // Skinned Matrix array buffer
            if (mesh->meshData->isSkinnedMesh) {
                mat4 skinnedMatrices[MAX_BONES];
                Skeleton *pSkeleton = mesh->meshData->skeleton;
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

            vao_bind(mesh->vao);

            MeshData *data = mesh->meshData;
            ui32 vertices  = data->vertexCount;
            ui32 indices   = data->indicesCount;

            ui16 drawMode = model->properties.drawMode;

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
          commandBuffer, 0, 1, &((Mesh *)model->mesh)->vkVBO->buffer, offsets);

        // Draw command
        vkCmdDraw(commandBuffer, ((Mesh *)model->mesh)->vkVBO->size, 1, 0, 0);
    }
}

static void
init(Entity e)
{
    if (!(ecs_has(e, C_TRANSFORM))) return;
    if (!(ecs_has(e, C_MODEL))) return;

    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
    struct ComponentModel *model         = ecs_get(e, C_MODEL);

    // TODO: stupid hack grabbing only scale.x...
    // mesh->model = load_obj(mesh->modelPath, transform->scale[0]);

    if (DEVICE_API_OPENGL) {
        if (!(Model *)model->pModel) {
            model->pModel = model_load_from_file(
              model->modelPath, transform->scale[0], FALSE);
        }
        model->mesh = ((Model *)model->pModel)->meshes[0];
        // send lightspace matrix from renderer to entity shader
        ShaderProgram *shader = ((Material *)model->material)->shaderProgram;
        shader_use(shader);
        glUniformMatrix4fv(
          glGetUniformLocation(shader->id, "u_LightSpaceMatrix"),
          1,
          GL_FALSE,
          (float *)e.ecs->renderer->lightSpaceMatrix);
        // TODO: send model matrix to shader
    } else if (DEVICE_API_VULKAN) {
        model->mesh = malloc(sizeof(Mesh));
        ((Mesh *)(model->mesh))->meshData =
          load_obj(model->modelPath, transform->scale[0]);

        VulkanDeviceContext *context = e.ecs->renderer->vulkanDevice;

        model->vkVBO = vulkan_vertex_buffer_create(
          context->physicalDevice,
          context->device,
          context->graphicsQueue,
          context->commandPool,
          ((Mesh *)model->mesh)->meshData->buffers.out,
          ((Mesh *)model->mesh)->meshData->buffers.outI * sizeof(float));
    }
}

static void
render(Entity e)
{
    if (!(ecs_has(e, C_TRANSFORM))) return;
    if (!(ecs_has(e, C_MODEL))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
    struct ComponentModel *model         = ecs_get(e, C_MODEL);

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
            debug_draw_skeleton(e.ecs->renderer->debugContext,
                                model->mesh->meshData->skeleton);
        }
#endif

#if DEBUG_DRAW_BOUNDS

        Model *pModel = model->pModel;
        for (int i = 0; i < pModel->meshesCount; i++) {
            Mesh *mesh = pModel->meshes[i];
            debug_draw_bounds(e.ecs->renderer->debugContext,
                              mesh->meshData->boundingBox,
                              transform->model);
        }
#endif

        // Regular Render
        (DEVICE_API_OPENGL)
          ? DrawModel(model, transform, FALSE, NULL, e.ecs->renderer)
          : DrawModel(model, transform, FALSE, cb, e.ecs->renderer);

    } else {
        (DEVICE_API_OPENGL)
          ? DrawModel(model, transform, TRUE, NULL, e.ecs->renderer)
          : DrawModel(model, transform, TRUE, cb, e.ecs->renderer);
    }
}

void
s_model_draw_init(ECS *ecs)
{
    //_ECS_DECL_COMPONENT(ecs, C_MODEL, sizeof(struct ComponentModel));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = (ECSSubscriber)render,
                          .update  = NULL,
                        }));
}
