#ifndef H_MESH
#define H_MESH

#include <util/sysdefs.h>
#include <glbuffer/glbuffer.h>

#include "texture.h"
#include "shader.h"

//#define material_create_s(x, ui32TextureCount, ...) _Generic((x))

typedef struct _material Material;
typedef struct _mesh Mesh;

struct _material {
    ShaderProgram *shaderProgram;
    Texture **textures;
    ui32 texturesCount;
};

struct _mesh {
    Material *material;
    const char *modelPath;
    VAO *vao;
    ui32 vertexCount;
};

Material *material_create(ShaderProgram *shader, ui32 textureCount, ...);
void material_use(Material *self);
void material_uniform(Material *self, char *value, ui32 type, void *data);

Mesh *mesh_create_obj(Material *material, const char* modelPath);
Mesh *mesh_create_primitive(Material *material, ui32 primitive);
void mesh_draw(Mesh *self);

#endif // H_MESH
