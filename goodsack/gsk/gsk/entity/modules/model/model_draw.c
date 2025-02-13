/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "model_draw.h"

#include "asset/import/loader_obj.h"
#include "entity/ecs.h"
#include "entity/modules/model/model.h"
#include "entity/modules/transform/transform.h"

#include "core/device/device.h"
#include "core/graphics/mesh/mesh.h"
#include "core/graphics/shader/shader.h"

#include "tools/debug/debug_draw_bounds.h"
#include "tools/debug/debug_draw_skeleton.h"

#define DEBUG_DRAW_SKELETON  0 // TODO: Does not draw with u_Model
#define DEBUG_DRAW_BOUNDS    0
#define CULLING_FOR_IMPORTED 0
#define CULLING_LOCAL        0

static void
__update_dynamic_uniforms(u32 shader_id,
                          u32 render_layer,
                          gsk_Mesh *mesh,
                          struct ComponentTransform *transform)
{
    // Skinned Matrix array buffer
    if (mesh->meshData->isSkinnedMesh)
    {
        mat4 skinnedMatrices[MAX_BONES];
        gsk_Skeleton *pSkeleton = mesh->meshData->skeleton;
        for (int i = 0; i < pSkeleton->jointsCount; i++)
        {
            glm_mat4_copy(pSkeleton->joints[i]->pose.mSkinningMatrix,
                          skinnedMatrices[i]);
        }

        glUniformMatrix4fv(glGetUniformLocation(shader_id, "u_SkinnedMatrices"),
                           pSkeleton->jointsCount,
                           GL_FALSE,
                           (float *)*skinnedMatrices);
    }
    mat4 newTranslation = GLM_MAT4_ZERO_INIT;
    glm_mat4_mul(mesh->localMatrix, transform->model, newTranslation);
    // Transform Uniform
    glUniformMatrix4fv(glGetUniformLocation(shader_id, "u_Model"),
                       1,
                       GL_FALSE,
                       (float *)newTranslation);

    // Set the correct camera layer
    glUniform1i(glGetUniformLocation(shader_id, "u_render_layer"),
                render_layer);
}

static void
__update_static_uniforms(u32 shader_id, gsk_Renderer *renderer)
{
    gsk_Scene *p_active_scene = renderer->sceneL[renderer->activeScene];

    // Light Space Matrix
    glUniformMatrix4fv(glGetUniformLocation(shader_id, "u_LightSpaceMatrix"),
                       1,
                       GL_FALSE,
                       (float *)renderer->lightSpaceMatrix);

    // Shadow options
    glUniform1i(glGetUniformLocation(shader_id, "u_pcfSamples"),
                renderer->shadowmapOptions.pcfSamples);
    glUniform1f(glGetUniformLocation(shader_id, "u_normalBiasMin"),
                renderer->shadowmapOptions.normalBiasMin);
    glUniform1f(glGetUniformLocation(shader_id, "u_normalBiasMax"),
                renderer->shadowmapOptions.normalBiasMax);

// Fog uniforms
#if 1
    glUniform1f(glGetUniformLocation(shader_id, "u_FogStart"),
                p_active_scene->fogOptions.fog_start);
    glUniform1f(glGetUniformLocation(shader_id, "u_FogEnd"),
                p_active_scene->fogOptions.fog_end);
    glUniform1f(glGetUniformLocation(shader_id, "u_FogDensity"),
                p_active_scene->fogOptions.fog_density);
    glUniform3fv(glGetUniformLocation(shader_id, "u_FogColor"),
                 1,
                 (float *)p_active_scene->fogOptions.fog_color);
#endif

    // SSAO Options
    glUniform1f(glGetUniformLocation(shader_id, "u_ssao_strength"),
                renderer->ssaoOptions.strength);

    // Total light count
    glUniform1i(glGetUniformLocation(shader_id, "u_total_lights"),
                p_active_scene->lighting_data.total_lights);

    // Light strength (Light 0 a.k.a. directional light)
    glUniform1f(glGetUniformLocation(shader_id, "u_light_strength"),
                p_active_scene->lighting_data.lights[0].strength);

    // Ambient options
    glUniform3fv(glGetUniformLocation(shader_id, "u_ambient_color_multiplier"),
                 1,
                 (float *)renderer->lightOptions.ambient_color_multiplier);

    glUniform1f(glGetUniformLocation(shader_id, "u_ambient_strength"),
                renderer->lightOptions.ambient_strength);
    glUniform1f(glGetUniformLocation(shader_id, "u_prefilter_strength"),
                renderer->lightOptions.prefilter_strength);
}

static void
DrawModel(struct ComponentModel *model,
          struct ComponentTransform *transform,
          u16 useOverrideMaterial, // Material from renderer
          u32 renderLayer,
          VkCommandBuffer commandBuffer,
          gsk_Renderer *renderer)
{
    gsk_Scene *p_active_scene = renderer->sceneL[renderer->activeScene];

    if (GSK_DEVICE_API_OPENGL)
    {
#if CULLING_FOR_IMPORTED
        // TODO: temporary solution for face culling.
        // This should be handled by a material property.
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CW);
#elif CULLING_LOCAL
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
#endif // CULLING_FOR_IMPORTED

        gsk_Model *pModel = model->pModel;
        for (int i = 0; i < pModel->meshesCount; i++)
        {
            // caching
            u8 is_new_material = FALSE;
            u8 is_new_shader   = FALSE;

            gsk_Mesh *mesh = pModel->meshes[i];
            gsk_Material *material;

            // Select Material
            if (mesh->usingImportedMaterial && !useOverrideMaterial)
            {
                material = mesh->materialImported;

            } else if (useOverrideMaterial)
            {
                // Check if shadow-casting is disabled
                if (renderer->currentPass == GskRenderPass_Shadowmap &&
                    model->cast_shadows == ECS_VAL_DISABLED)
                {
                    return;
                }

                if (mesh->meshData->isSkinnedMesh)
                {
                    material = renderer->explicitMaterial_skinned;
                } else
                {
                    material = renderer->explicitMaterial;
                }
            }

            else
            {
                material = model->material;
            }

            // Check Material caching (for texture loading)
            if (material != renderer->p_prev_material)
            {
                gsk_material_load_textures(material);
                renderer->p_prev_material = material;
                is_new_material           = TRUE;
            }
            // Check Shader caching (for uniform updating)
            if (material->shaderProgram->id != renderer->prev_shader_id)
            {
                gsk_shader_use(material->shaderProgram);
                renderer->prev_shader_id = material->shaderProgram->id;
                is_new_shader            = TRUE;
            }

            // TESTING for normal-map in G-Buffer
            // TODO: Breaks when normal-map doesn't exist
            // TODO: Refactor this block
            if (renderer->currentPass == GskRenderPass_GBuffer)
            {
                if (mesh->usingImportedMaterial &&
                    mesh->materialImported->texturesCount > 1)
                {
                    texture_bind(mesh->materialImported->textures[1], 10);
                } else if (!mesh->usingImportedMaterial)
                {
                    gsk_Material *mat = model->material;
                    if (mat->texturesCount >= 2)
                    {
                        texture_bind(mat->textures[1], 10);
                    }
                }
            }

            u32 shader_id = material->shaderProgram->id;

            __update_dynamic_uniforms(shader_id, renderLayer, mesh, transform);

            if (is_new_shader == TRUE)
            {
                __update_static_uniforms(shader_id, renderer);
                //__update_culling();
            }

            // bind VAO
            gsk_gl_vertex_array_bind(mesh->vao);

            gsk_MeshData *data = mesh->meshData;
            u32 vertices       = data->vertexCount;
            u32 indices        = data->indicesCount;

            // u16 drawMode = model->properties.drawMode;

            GskMeshPrimitiveType_ primitive = mesh->meshData->primitive_type;
            GLenum gl_prim                  = GL_TRIANGLES;

            switch (primitive)
            {
            case GskMeshPrimitiveType_Triangle: gl_prim = GL_TRIANGLES; break;
            case GskMeshPrimitiveType_Quad: gl_prim = GL_QUADS; break;
            case GskMeshPrimitiveType_Poly: gl_prim = GL_POLYGON; break;
            case GskMeshPrimitiveType_Fan: gl_prim = GL_TRIANGLE_FAN; break;
            default: break;
            }

            if (data->has_indices)
            {
                glDrawElements(gl_prim, indices, GL_UNSIGNED_INT, NULL);
            } else
            {
                glDrawArrays(gl_prim, 0, vertices);
            }
        }

#if CULLING_LOCAL
        // Handle model-draw end | culling
        glDisable(GL_CULL_FACE);
#endif

    } else if (GSK_DEVICE_API_VULKAN)
    {

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
        vkCmdBindVertexBuffers(commandBuffer,
                               0,
                               1,
                               &((gsk_Mesh *)model->mesh)->vkVBO->buffer,
                               offsets);

        // Draw command
        vkCmdDraw(
          commandBuffer, ((gsk_Mesh *)model->mesh)->vkVBO->size, 1, 0, 0);
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

    if (GSK_DEVICE_API_OPENGL)
    {
        if (!(gsk_Model *)model->pModel)
        {
            model->pModel = gsk_model_load_from_file(
              model->modelPath, transform->scale[0], FALSE);
        }

        // TODO: Duplicate model here (for skinned-mesh / animator)
        // TODO: we probably dont want model->mesh. Maybe model->skinned_mesh?
        model->mesh = ((gsk_Model *)model->pModel)->meshes[0];

        // TODO: rework defaults
        if (model->cast_shadows && model->cast_shadows != 2)
        {
            model->cast_shadows = 1;
        }

    } else if (GSK_DEVICE_API_VULKAN)
    {
        model->mesh = malloc(sizeof(gsk_Mesh));
        ((gsk_Mesh *)(model->mesh))->meshData =
          gsk_load_obj(model->modelPath, transform->scale[0]);

        VulkanDeviceContext *context = e.ecs->renderer->vulkanDevice;

        model->vkVBO = vulkan_vertex_buffer_create(
          context->physicalDevice,
          context->device,
          context->graphicsQueue,
          context->commandPool,
          ((gsk_Mesh *)model->mesh)->meshData->mesh_buffers[0].p_buffer,
          ((gsk_Mesh *)model->mesh)->meshData->mesh_buffers[0].buffer_size);
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
    if (gsk_ecs_has(e, C_RENDERLAYER))
    {
        renderLayer =
          (u32)((struct ComponentRenderLayer *)gsk_ecs_get(e, C_RENDERLAYER))
            ->renderLayer;
    }

    GskRenderPass pass = e.ecs->renderer->currentPass;

    // TODO: get lightspace matrix

    VkCommandBuffer cb;
    if (GSK_DEVICE_API_VULKAN)
    {
        cb = e.ecs->renderer->vulkanDevice
               ->commandBuffers[e.ecs->renderer->vulkanDevice->currentFrame];
    }

    if (pass == GskRenderPass_Lighting)
    {
#if DEBUG_DRAW_SKELETON
        // draw skeleton
        if (((gsk_Mesh *)model->mesh)->meshData->isSkinnedMesh)
        {
            gsk_debug_draw_skeleton(
              e.ecs->renderer->debugContext,
              ((gsk_Mesh *)model->mesh)->meshData->skeleton);
        }
#endif

#if DEBUG_DRAW_BOUNDS

        gsk_Model *pModel = model->pModel;
        for (int i = 0; i < pModel->meshesCount; i++)
        {
            gsk_Mesh *mesh = pModel->meshes[i];
            gsk_debug_draw_bounds(e.ecs->renderer->debugContext,
                                  mesh->meshData->boundingBox,
                                  transform->model);
        }
#endif

        // Regular Render
        (GSK_DEVICE_API_OPENGL)
          ? DrawModel(
              model, transform, FALSE, renderLayer, NULL, e.ecs->renderer)
          : DrawModel(
              model, transform, FALSE, renderLayer, cb, e.ecs->renderer);

    } else if (pass != GskRenderPass_Skybox)
    {
        (GSK_DEVICE_API_OPENGL)
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
