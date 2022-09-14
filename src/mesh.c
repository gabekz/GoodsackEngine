#include "mesh.h"
#include <stdlib.h>

#include <loaders/loader_obj.h>
#include "primitives.h"
#include "gfx.h"

Mesh *mesh_create_obj(Material *material, const char* modelPath,
        ui32 cullEnable, ui32 cullFace, ui32 cullFrontFace) {

    Model *model = load_obj(modelPath);

    Mesh *ret = malloc(sizeof(Mesh));
    ret->model = model;

    ret->drawingMode   = DRAW_MODE_ARRAYS;
    ret->renderingMode = GL_TRIANGLES;

    ret->material = material;

    ret->cullEnable = cullEnable;
    ret->cullFace = cullFace;
    ret-> cullFrontFace = cullFrontFace;
    return ret;
}

Mesh *mesh_create_primitive(Material *material, ui32 primitive,
        ui32 cullEnable, ui32 cullFace, ui32 cullFrontFace) {
    Mesh *ret = malloc(sizeof(Mesh));

    float *points;
    ui32 pointsSize = (3 * 8) * sizeof(float);
    switch(primitive) {
        case PRIMITIVE_PLANE:
            //points = prim_vert_plane();
            //pointsSize = PRIMITIVE_SIZE_PLANE;
            //ret->drawingMode = GL_TRIANGLES;
            break;
        case PRIMITIVE_CUBE:
            points = prim_vert_cube(0.03f);
            pointsSize = PRIMITIVE_SIZE_CUBE;
            ret->drawingMode = DRAW_MODE_ARRAYS;
            ret->renderingMode = GL_TRIANGLE_STRIP;
            break;
        case PRIMITIVE_PYRAMID:
            //points = prim_vert_pyramid();
            //pointsSize = PRIMITIVE_SIZE_PYRAMID;
            break;
        default:
            return NULL;
            break;
    }
    
    VAO *vao = vao_create();
    vao_bind(vao);
    VBO *vbo = vbo_create(points, 6 * 4 * 3 * sizeof(float));
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vbo);

    Model *model = malloc(sizeof(Model));
    model->vao = vao;
    model->vertexCount = 24;
    model->modelPath = NULL;

    ret->model = model;
    ret->material = material;
    ret->cullEnable = cullEnable;
    ret->cullFace = cullFace;
    ret-> cullFrontFace = cullFrontFace;

    return ret;
}

void mesh_draw(Mesh *self) {
    if(self->cullEnable == GL_TRUE) {
        glEnable(GL_CULL_FACE);
        glCullFace(self->cullFace);
        glFrontFace(self->cullFrontFace);
    }
    else {
        glDisable(GL_CULL_FACE);
    }

    material_use(self->material);
    vao_bind(self->model->vao);

    ui32 mode = self->drawingMode;
    ui32 vertices = self->model->vertexCount;
    switch(mode) {
        case DRAW_MODE_ARRAYS:
            glDrawArrays(self->renderingMode, 0, vertices);
            break;
        case DRAW_MODE_ELEMENTS:
            break;
    }
}
