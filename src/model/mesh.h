#ifndef H_MESH
#define H_MESH

#include <util/sysdefs.h>
#include <glbuffer/glbuffer.h>

#include <model/material.h>
#include "texture.h"
#include "shader.h"

typedef struct _mesh Mesh;
typedef struct _model Model;

struct _mesh {
    Material *material;
    Model *model;

    ui32 cullEnable, cullFace, cullFrontFace;
    ui32 drawingMode, renderingMode;
};

struct _model {
    VAO* vao;
    ui32 vertexCount;
    ui32 indicesCount;
    const char *modelPath;
};

Mesh *mesh_create_obj(Material *material, const char* modelPath, float scale,
        ui32 cullEnable, ui32 cullFace, ui32 cullFrontFace);

Mesh *mesh_create_primitive(Material *material, ui32 primitive, float scale,
        ui32 cullEnable, ui32 cullFace, ui32 cullFrontFace);

void mesh_draw(Mesh *self);

#endif // H_MESH
