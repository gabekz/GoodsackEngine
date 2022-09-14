#include "mesh.h"
#include <stdlib.h>

#include <loaders/loader_obj.h>
#include "primitives.h"
#include "gfx.h"

Mesh *mesh_create_obj(Material *material, const char* modelPath) {
    Mesh *ret = malloc(sizeof(Mesh));
    Model *model = load_obj(modelPath);

    ret->vertexCount = model->indicesCount;

    ret->material = material;
    ret->vao = model->vao;
    return ret;
}

Mesh *mesh_create_primitive(Material *material, ui32 primitive) {
    Mesh *ret = malloc(sizeof(Mesh));

    float *points;
    ui32 pointsSize = (3 * 8) * sizeof(float);
    switch(primitive) {
        case PRIMITIVE_PLANE:
            points = prim_vert_plane();
            pointsSize = PRIMITIVE_SIZE_PLANE;
            break;
        case PRIMITIVE_CUBE:
            points = prim_vert_cube(0.03f);
            pointsSize = PRIMITIVE_SIZE_CUBE;
            break;
        case PRIMITIVE_PYRAMID:
            points = prim_vert_pyramid();
            pointsSize = PRIMITIVE_SIZE_PYRAMID;
            break;
        default:
            return NULL;
            break;
    }
    
    VAO *vao = vao_create();
    vao_bind(vao);
    VBO *vbo = vbo_create(points, 3 * 8 * sizeof(float));
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vbo);

    ret->material = material;
    ret->modelPath = NULL;
    ret->vao = vao;
    ret->vertexCount = 24;


    return ret;
}

void mesh_draw(Mesh *self) {
    material_use(self->material);
    vao_bind(self->vao);
    glDrawArrays(GL_TRIANGLES, 0, self->vertexCount);
}
