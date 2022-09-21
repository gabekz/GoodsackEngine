#ifndef H_MATERIAL
#define H_MATERIAL

#include <util/sysdefs.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>
#include "texture.h"
#include "shader.h"

typedef struct _material Material;

struct _material {
    ShaderProgram *shaderProgram;
    Texture **textures;
    ui32 texturesCount;
};

Material *material_create(
    ShaderProgram *shader, const char *shaderPath, ui32 textureCount, ...);
void material_use(Material *self);

#endif // H_MATERIAL
