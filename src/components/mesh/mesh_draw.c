#include "mesh_draw.h"
#include "mesh.h"

#include <components/transform/transform.h>

#include <core/shader/shader.h>
#include <ecs/ecs.h>
#include <import/loader_obj.h>
#include <model/model.h>

#include <core/api/device.h>

static void DrawMesh(
        struct ComponentMesh *mesh,
        struct ComponentTransform *transform,
        Material *material)
{
    if(DEVICE_API_OPENGL) {

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
        1, GL_FALSE, (float *)transform->mvp.model);

        vao_bind(mesh->model->vao);

        ModelData *data = mesh->model->modelData;
        ui32 vertices = data->vertexCount;
        ui32 indices  = data->indicesCount;

        ui16 drawMode = mesh->properties.drawMode;

        switch(drawMode) {
            case DRAW_ELEMENTS:
                glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, NULL);
            break;
            case DRAW_ARRAYS:
            default:
                glDrawArrays(GL_TRIANGLES, 0, vertices);
            break;
        }

    }
    else if(DEVICE_API_VULKAN) {
    #if 0
        // Bind transform Model (MVP) and Position (vec3) descriptor set
        vkCmdBindDescriptorSets(
                *commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                context->pipelineDetails->pipelineLayout, 0, 1,
                &context->descriptorSets[context->currentFrame], 0, NULL);
        // Bind texture descriptor set
        vkCmdBindDescriptorSets(
                *commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                context->pipelineDetails->pipelineLayout, 0, 1,
                &context->descriptorSets[context->currentFrame], 0, NULL);
        // Bind Vertex/Index buffers
        vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);

        // Draw command
        vkCmdDraw(*commandBuffer, context->vertexBuffer->size, 1, 0, 0);
    #endif
    }
}


static void init(Entity e) {
    if(!(ecs_has(e, C_TRANSFORM))) return;
    if(!(ecs_has(e, C_MESH))) return;

    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
    struct ComponentMesh *mesh = ecs_get(e, C_MESH);

    // TODO: stupid hack grabbing only scale.x...
    //mesh->model = load_obj(mesh->modelPath, transform->scale[0]);

    mesh->model = model_assemble(mesh->modelPath, transform->scale[0]);

    // send lightspace matrix from renderer to entity shader
    ShaderProgram *shader = mesh->material->shaderProgram;
    shader_use(shader);
    glUniformMatrix4fv(
        glGetUniformLocation(shader->id, "u_LightSpaceMatrix"),
        1, GL_FALSE, (float *)e.ecs->renderer->lightSpaceMatrix);

    // TODO: send model matrix to shader
}

static void render(Entity e) {
    if(!(ecs_has(e, C_TRANSFORM))) return;
    if(!(ecs_has(e, C_MESH))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
    struct ComponentMesh *mesh = ecs_get(e, C_MESH);

    RenderPass pass = e.ecs->renderer->currentPass;

    // TODO: get lightspace matrix

    if(pass == REGULAR) {
        DrawMesh(mesh, transform, mesh->material);
        return;
    }
    Material *override = e.ecs->renderer->explicitMaterial;
    DrawMesh(mesh, transform, override);
}

void s_mesh_draw_init(ECS *ecs) {
    ecs_component_register(ecs, C_MESH, sizeof(struct ComponentMesh));
    ecs_system_register(ecs, ((ECSSystem){
        .init       = (ECSSubscriber) init,
        .destroy    = NULL,
        .render     = (ECSSubscriber) render,
        .update     = NULL,
    }));
}
