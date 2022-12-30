#include "material.h"

#include <stdio.h>
#include <stdarg.h>

#include <util/gfx.h>
#include <util/logger.h>

#include <core/api/device.h>
#include <core/texture/texture.h>
#include <core/shader/shader.h>

Material *material_create(
    ShaderProgram *shader, const char *shaderPath, ui32 textureCount, ...) {

    Material *ret = malloc(sizeof(Material));
    if(shader) {
        ret->shaderProgram = shader;
    } 
    else if (shaderPath != "" || shaderPath != NULL) {
        ret->shaderProgram = shader_create_program(shaderPath);
    }
    else {
        LOG_ERROR("You need to pass either a ShaderProgram or a valid path");
        return NULL;
    }

    //ret->textures = textures;
    ret->texturesCount = textureCount;

    if(textureCount <= 0) {
        ret->texturesCount = 0;
        return ret;
    }

    va_list ap;
    va_start(ap, textureCount);
    va_end(ap);
    Texture **textures = malloc(textureCount * sizeof(Texture*));
    for(int i = 0; i < textureCount; i++) {
        *(textures+i) = va_arg(ap, Texture*);
    }
    ret->textures = textures;
    ret->texturesCount = textureCount;
    return ret;

    // TODO: Create descriptor set
}

void material_use(Material *self) {
    if(DEVICE_API_OPENGL) {
        if(self->texturesCount > 0) {
            for(int i = 0; i < self->texturesCount; i++) {
                texture_bind(self->textures[i], i);
            }
        }
        shader_use(self->shaderProgram);
    }
    else if(DEVICE_API_VULKAN) {
        LOG_DEBUG("Material not implemented for Vulkan");

        // Bind Pipeline here? Probably.
        // TODO: Bind image descriptor set HERE
    }
}
