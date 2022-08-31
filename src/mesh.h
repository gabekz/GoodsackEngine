#ifndef H_MESH
#define H_MESH

#include <util/sysdefs.h>
#include <glbuffer/glbuffer.h>

typedef struct _mesh Mesh;

struct _mesh {
    ui32 shaderId;
    VAO *vao;
    const char *modelPath;
};

Mesh *mesh_create(const char *modelPath, ui32 shaderId);
void mesh_draw(Mesh *self);

#endif // H_MESH
