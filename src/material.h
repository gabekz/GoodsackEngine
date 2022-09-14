#ifndef H_MATERIAL
#define H_MATERIAL

#include <util/sysdefs.h>
#include "texture.h"
#include "shader.h"

typedef struct _material Material;

struct _material {
    ShaderProgram *shaderProgram;
    Texture **textures;
    ui32 texturesCount;
};

Material *material_create(ShaderProgram *shader, ui32 textureCount, ...);
void material_use(Material *self);
void material_uniform(Material *self, char *value, ui32 type, void *data);

#endif // H_MATERIAL
