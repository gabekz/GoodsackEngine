#include "model_draw.h"

#include <ecs/builtin/model/model.h>
#include <ecs/builtin/transform/transform.h>

#include <asset/import/loader_obj.h>
#include <core/graphics/mesh/mesh.h>
#include <core/graphics/shader/shader.h>
#include <ecs/ecs.h>

#include <core/device/device.h>

#include <tools/debug/debug_draw_skeleton.h>

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

        // Enable material + shaders
        material_use(material);

        // Transform Uniform
        glUniformMatrix4fv(
          glGetUniformLocation(material->shaderProgram->id, "u_Model"),
          1,
          GL_FALSE,
          (float *)transform->mvp.model);

        vao_bind(model->mesh->vao);

        MeshData *data = model->mesh->meshData;
        ui32 vertices  = data->vertexCount;
        ui32 indices   = data->indicesCount;

        ui16 drawMode = model->properties.drawMode;

        switch (drawMode) {
        case DRAW_ELEMENTS:
            glDrawElements(GL_LINES, indices, GL_UNSIGNED_INT, NULL);
            break;
        case DRAW_ARRAYS:
        default: glDrawArrays(GL_TRIANGLES, 0, vertices); break;
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
        model->mesh = mesh_assemble(model->modelPath, transform->scale[0]);
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

    // draw skeleton
    if (model->mesh->meshData->isSkinnedMesh) {
        debug_draw_skeleton(e.ecs->renderer->debugContext,
                            model->mesh->meshData->skeleton);
    }

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
