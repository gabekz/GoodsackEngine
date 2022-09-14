#ifndef H_MESH
#define H_MESH

#include <util/sysdefs.h>
#include <glbuffer/glbuffer.h>

#include "material.h"
#include "texture.h"
#include "shader.h"

//#define material_create_s(x, ui32TextureCount, ...) _Generic((x))

typedef struct _mesh Mesh;

struct _mesh {
    Material *material;
    const char *modelPath;
    VAO *vao;
    ui32 vertexCount;
};


Mesh *mesh_create_obj(Material *material, const char* modelPath);
Mesh *mesh_create_primitive(Material *material, ui32 primitive);
void mesh_draw(Mesh *self);

#endif // H_MESH
