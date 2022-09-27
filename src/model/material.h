#ifndef H_MATERIAL
#define H_MATERIAL

#include <util/sysdefs.h>

#include <core/texture.h>
#include <core/shader.h>

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
