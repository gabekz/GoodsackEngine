#include "material.h"
#include <stdio.h>
#include <stdarg.h>

#include <gfx.h>

Material *material_create(ShaderProgram *shader, ui32 textureCount, ...) {
    Material *ret = malloc(sizeof(Material));
    ret->shaderProgram = shader;

    //ret->textures = textures;
    ret->texturesCount = textureCount;

    if(textureCount <= 0) {
        ret->texturesCount = 0;
        return ret;
    }

    va_list ap;
    va_start(ap, textureCount);
    va_end(ap);
    Texture **textures = malloc(textureCount * sizeof(Texture));
    for(int i = 0; i < textureCount; i++) {
        *(textures+i) = va_arg(ap, Texture*);
    }
    ret->textures = textures;
    ret->texturesCount = textureCount;
    return ret;
}

void material_use(Material *self) {
    if(self->texturesCount > 0) {
        for(int i = 0; i < self->texturesCount; i++) {
            texture_bind(self->textures[i], i);
        }
    }
    shader_use(self->shaderProgram);
}
