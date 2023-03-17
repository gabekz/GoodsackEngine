#include "model_draw.h"

#include <ecs/builtin/model/model.h>
#include <ecs/builtin/transform/transform.h>

#include <asset/import/loader_obj.h>
#include <core/graphics/mesh/mesh.h>
#include <core/graphics/shader/shader.h>
#include <ecs/ecs.h>

#include <core/device/device.h>

#include <tools/debug/debug_draw_skeleton.h>

// #define DEBUG_DRAW_SKELETON

static void
DrawModel(struct ComponentModel *model,
          struct ComponentTransform *transform,
          Material *material,
          VkCommandBuffer commandBuffer)
{
    if (DEVICE_API_OPENGL) {

// Handle culling
#if 0
        if((mesh->properties.cullMode | CULL_DISABLED)) {
            printf("Disable culling");
        }
#endif
        for (int i = 0; i < model->pModel->meshesCount; i++) {
            Mesh *mesh = model->pModel->meshes[i];

            // Enable material + shaders
            if (mesh->usingImportedMaterial) {
                material = mesh->materialImported;
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
            glm_mat4_mul(
              mesh->localMatrix, transform->mvp.model, newTranslation);
            // Transform Uniform
            glUniformMatrix4fv(
              glGetUniformLocation(material->shaderProgram->id, "u_Model"),
              1,
              GL_FALSE,
              (float *)newTranslation);

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
          commandBuffer, 0, 1, &model->mesh->vkVBO->buffer, offsets);

        // Draw command
        vkCmdDraw(commandBuffer, model->mesh->vkVBO->size, 1, 0, 0);
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
        model->pModel =
          model_load_from_file(model->modelPath, transform->scale[0]);
        model->mesh = model->pModel->meshes[0];
        // send lightspace matrix from renderer to entity shader
        ShaderProgram *shader = model->material->shaderProgram;
        shader_use(shader);
        glUniformMatrix4fv(
          glGetUniformLocation(shader->id, "u_LightSpaceMatrix"),
          1,
          GL_FALSE,
          (float *)e.ecs->renderer->lightSpaceMatrix);
        // TODO: send model matrix to shader
    } else if (DEVICE_API_VULKAN) {
        model->mesh           = malloc(sizeof(Mesh));
        model->mesh->meshData = load_obj(model->modelPath, transform->scale[0]);

        VulkanDeviceContext *context = e.ecs->renderer->vulkanDevice;

        model->mesh->vkVBO = vulkan_vertex_buffer_create(
          context->physicalDevice,
          context->device,
          context->graphicsQueue,
          context->commandPool,
          model->mesh->meshData->buffers.out,
          model->mesh->meshData->buffers.outI * sizeof(float));
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

#if defined(DEBUG_DRAW_SKELETON)
    // draw skeleton
    if (model->mesh->meshData->isSkinnedMesh) {
        debug_draw_skeleton(e.ecs->renderer->debugContext,
                            model->mesh->meshData->skeleton);
    }
#endif

    if (pass == REGULAR) {
        (DEVICE_API_OPENGL) ? DrawModel(model, transform, model->material, NULL)
                            : DrawModel(model, transform, model->material, cb);
        return;
    }
    Material *override = e.ecs->renderer->explicitMaterial;

    (DEVICE_API_OPENGL) ? DrawModel(model, transform, override, NULL)
                        : DrawModel(model, transform, override, cb);
}

void
s_model_draw_init(ECS *ecs)
{
    ecs_component_register(ecs, C_MODEL, sizeof(struct ComponentModel));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = (ECSSubscriber)render,
                          .update  = NULL,
                        }));
}
