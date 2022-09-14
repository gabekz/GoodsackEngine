#include "mesh.h"
#include <stdlib.h>

#include <loaders/loader_obj.h>
#include <model/primitives.h>
#include "gfx.h"

Mesh *mesh_create_obj(Material *material, const char* modelPath,
        ui32 cullEnable, ui32 cullFace, ui32 cullFrontFace) {

    Model *model = load_obj(modelPath);

    Mesh *ret = malloc(sizeof(Mesh));
    ret->model = model;

    ret->drawingMode = 
        (model->indicesCount <= 0) ? DRAW_MODE_ARRAYS: DRAW_MODE_ELEMENTS;
    ret->renderingMode = GL_TRIANGLES;

    ret->material = material;

    ret->cullEnable = cullEnable;
    ret->cullFace = cullFace;
    ret-> cullFrontFace = cullFrontFace;
    return ret;
}

static float* FillFloat(float data[], ui32 size, float scale) {
    float *ret = malloc(size * sizeof(float));
    for(int i = 0; i < size; i++) {
        ret[i] = data[i] * scale;
    }
    return ret;
}
static ui32* FillInt(ui32 data[], ui32 size) {
    ui32 *ret = malloc(size * sizeof(ui32));
    for(int i = 0; i < size; i++) {
        ret[i] = data[i];
    }
    return ret;
}

Mesh *mesh_create_primitive(Material *material, ui32 primitive, float scale,
        ui32 cullEnable, ui32 cullFace, ui32 cullFrontFace) {
    Mesh *ret = malloc(sizeof(Mesh));

    float *points;
    ui32 pointsSize;

    ui32 *indices;
    ui32 indicesSize;

    switch(primitive) {
        case PRIMITIVE_PLANE:
            //points = prim_vert_plane();
            //pointsSize = PRIMITIVE_SIZE_PLANE;
            //ret->drawingMode = GL_TRIANGLES;
            break;
        case PRIMITIVE_CUBE:

            pointsSize = PRIM_SIZ_V_CUBE;
            points = FillFloat(PRIM_ARR_V_CUBE, pointsSize, scale);

            indicesSize = PRIM_SIZ_I_CUBE;
            indices = FillInt(PRIM_ARR_I_CUBE, indicesSize);

            ret->drawingMode = DRAW_MODE_ELEMENTS;
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
    VBO *vbo = vbo_create(points, pointsSize * sizeof(float));
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vbo);
    if(indicesSize >= 0) {
        IBO *ibo = ibo_create(indices, indicesSize * sizeof(ui32));
    }

    Model *model = malloc(sizeof(Model));
    model->vao = vao;
    model->vertexCount = pointsSize;
    model->indicesCount = indicesSize;
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
    ui32 indices = self->model->indicesCount;
    switch(mode) {
        case DRAW_MODE_ARRAYS:
            glDrawArrays(self->renderingMode, 0, vertices);
            break;
        case DRAW_MODE_ELEMENTS:
            glDrawElements(self->renderingMode, indices, GL_UNSIGNED_INT, NULL);
            break;
    }
}
