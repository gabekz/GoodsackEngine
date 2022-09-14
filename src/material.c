#include "material.h"
#include <stdlib.h>
#include <stdarg.h>

#include <gfx.h>

Material *material_create(ShaderProgram *shader, ui32 textureCount, ...) {
    Material *ret = malloc(sizeof(Material));

    va_list ap;
    va_start(ap, textureCount);
    va_end(ap);

    //Texture *textures[textureCount]; 
    //for(int i = 0; i < textureCount; i++) {
        // TODO: reallocate here
    //    textures[i] = va_arg(ap, Texture*);
    //}

    //ret->textures = textures;
    ret->texturesCount = textureCount;
    ret->shaderProgram = shader;

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

void material_uniform(Material *self, char *value, ui32 type, void *data) {
    shader_use(self->shaderProgram);
    ui32 location = glGetUniformLocation(self->shaderProgram->id, value);

    glUniformMatrix4fv(location, 1, GL_FALSE, data);
}
