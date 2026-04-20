/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "material.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "util/filesystem.h"
#include "util/gfx.h"
#include "util/logger.h"

#include "core/device/device.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

#include "runtime/gsk_runtime_wrapper.h"

#include "core/drivers/vulkan/vulkan.h"

#include "asset/asset.h"
#include "asset/asset_cache.h"
#include "asset/asset_gcfg.h"

#define FALLBACK_TEXTURE_URI "gsk://textures/defaults/missing_1.png"

gsk_Material *
gsk_material_create(gsk_ShaderProgram *shader,
                    const char *shaderPath,
                    u32 textureCount,
                    ...)
{

    gsk_Material *ret = malloc(sizeof(gsk_Material));
    if (shader)
    {
        ret->shaderProgram = shader;
    } else if (shaderPath != NULL)
    {
        char uri[GSK_FS_MAX_PATH];
        gsk_filesystem_path_to_uri(shaderPath, uri);
        ret->shaderProgram = GSK_ASSET(uri);
    } else
    {
        LOG_ERROR(
          "You need to pass either a gsk_ShaderProgram or a valid path");
        return NULL;
    }

    // ret->textures = textures;
    ret->texturesCount = textureCount;

    if (textureCount <= 0)
    {
        ret->texturesCount = 0;
        return ret;
    }

    va_list ap;
    va_start(ap, textureCount);
    gsk_Texture **textures = malloc(textureCount * sizeof(gsk_Texture *));
    for (int i = 0; i < textureCount; i++)
    {
        gsk_Texture *p_tex = va_arg(ap, gsk_Texture *);

        if (p_tex == NULL)
        {
            LOG_WARN("texture (%d) on material %p is invalid - using fallback",
                     i,
                     ret);

            p_tex = GSK_ASSET(FALLBACK_TEXTURE_URI);
        }

        *(textures + i) = p_tex;
    }
    ret->textures      = textures;
    ret->texturesCount = textureCount;
    va_end(ap);
    return ret;

    // TODO: Create descriptor set
}
gsk_Material *
gsk_material_create_from_gcfg(gsk_GCFG *p_gcfg)
{
    gsk_ShaderProgram *p_shader = NULL;
    gsk_Material *p_material    = NULL;

    // need to grab the shader
    for (int i = 0; i < p_gcfg->list_items.list_next; i++)
    {
        gsk_GCFGItem *item = array_list_get_at_index(&p_gcfg->list_items, i);
        if (!strcmp(item->key, "shader"))
        {
            // get correct cache based on shader path
            gsk_AssetCache *p_cache = gsk_runtime_get_asset_cache(item->value);

            if (hash_table_has(&p_cache->asset_table, item->value) == FALSE)
            {
                LOG_ERROR("Shader asset is not cached! (%s)", item->value);
            }

            p_shader = (gsk_ShaderProgram *)GSK_ASSET(item->value);
        }
    }
    if (p_shader == NULL)
    {
        // TODO: get filename from gcfg
        LOG_ERROR("GCFG Material does not have a shader reference.");
        return NULL;
    }

    p_material = gsk_material_create(p_shader, NULL, 0);

    // attach textures
    for (int i = 0; i < p_gcfg->list_items.list_next; i++)
    {
        gsk_GCFGItem *item = array_list_get_at_index(&p_gcfg->list_items, i);
        if (!strcmp(item->key, "texture"))
        {
            // get correct cache based on texture path
            gsk_AssetCache *p_cache = gsk_runtime_get_asset_cache(item->value);

            // load and add texture

            gsk_Texture *p_tex = (gsk_Texture *)GSK_ASSET(item->value);

            gsk_material_add_texture(p_material, p_tex);
        }
    }

    return p_material;
}

void
gsk_material_load_textures(gsk_Material *self)
{
    if (GSK_DEVICE_API_OPENGL)
    {
        if (self->texturesCount > 0)
        {
            for (int i = 0; i < self->texturesCount; i++)
            {
                texture_bind(self->textures[i], i);
            }
        }

    } else if (GSK_DEVICE_API_VULKAN)
    {
#if 0
        VulkanDeviceContext *p_context =
          gsk_runtime_get_renderer()->vulkanDevice;
        VkDescriptorSet *sets = p_context->descriptorSets;

        // LOG_DEBUG("Material not implemented for Vulkan");

        // Bind Pipeline here? Probably.
        // TODO: Bind image descriptor set HERE
        VkDescriptorImageInfo imageInfo = {
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          .imageView   = self->textures[0]->vulkan.textureImageView,
          .sampler     = self->textures[0]->vulkan.textureSampler,
        };

        for (int i = 0; i < 2; i++)
        {

            u32 descriptorWritesCount = 1;
            VkWriteDescriptorSet descriptorWrites[1];

            descriptorWrites[0] = (VkWriteDescriptorSet) {
              .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              .dstSet          = sets[i],
              .dstBinding      = 1,
              .dstArrayElement = 0,

              .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              .descriptorCount = 1,

              .pImageInfo = &imageInfo,

            };

            vkUpdateDescriptorSets(
              p_context->device, 1, descriptorWrites, 0, NULL);
    }
#endif
    }
}

void
gsk_material_use(gsk_Material *self)
{
    gsk_material_load_textures(self);
    if (GSK_DEVICE_API_OPENGL) { gsk_shader_use(self->shaderProgram); }

    // update previous material on renderer
    gsk_runtime_get_renderer()->p_prev_material = self;
}

void
gsk_material_add_texture(gsk_Material *self, gsk_Texture *texture)
{
    if (self->texturesCount == 0)
    {
        self->textures      = malloc(sizeof(gsk_Texture *));
        self->textures[0]   = texture;
        self->texturesCount = 1;
        return;
    }
    self->texturesCount = self->texturesCount += 1;
    self->textures =
      realloc(self->textures, self->texturesCount * sizeof(gsk_Texture *));
    self->textures[self->texturesCount - 1] = texture;
}
