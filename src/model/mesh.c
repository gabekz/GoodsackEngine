#include <model/mesh.h>
#include <stdlib.h>

#include <loaders/loader_obj.h>
#include <model/primitives.h>
#include "gfx.h"

Mesh *mesh_create_obj(Material *material, const char* modelPath, float scale,
        ui32 cullEnable, ui32 cullFace, ui32 cullFrontFace) {

    Model *model = load_obj(modelPath, scale);

    Mesh *ret = malloc(sizeof(Mesh));
    ret->model = model;

    // default model matrix TODO: Move to ecs?
    //mat4 modelMatrix = GM_MAT4_IDENTITY_INIT;
    //model_set_matrix(modelMatrix);

    // TODO: FIX THIS when IBO implemented for .obj files.
    //ret->drawingMode = 
    //    (model->indicesCount <= 0) ? DRAW_MODE_ARRAYS: DRAW_MODE_ELEMENTS;
    ret->drawingMode = DRAW_MODE_ARRAYS;

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

    ui32 pointsSize  = 0;
    ui32 indicesSize = 0;

    float *points;
    ui32 *indices;

    VAO *vao = vao_create();
    vao_bind(vao);
    VBO *vbo;

    switch(primitive) {
        case PRIMITIVE_PLANE:
            pointsSize  = PRIM_SIZ_V_PLANE;
            points = FillFloat(PRIM_ARR_V_PLANE, pointsSize, scale);
            vbo = vbo_create(points, pointsSize * sizeof(float));
            vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
            vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
            ret->drawingMode = DRAW_MODE_ARRAYS;
            ret->renderingMode = GL_TRIANGLE_STRIP;
            break;
        case PRIMITIVE_CUBE:
            pointsSize  = PRIM_SIZ_V_CUBE;
            indicesSize = PRIM_SIZ_I_CUBE;
            points  = FillFloat(PRIM_ARR_V_CUBE, pointsSize, scale);
            indices = FillInt(PRIM_ARR_I_CUBE, indicesSize);
            vbo = vbo_create(points, pointsSize * sizeof(float));
            vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
            ret->drawingMode = DRAW_MODE_ELEMENTS;
            ret->renderingMode = GL_TRIANGLE_STRIP;
            break;
        case PRIMITIVE_PYRAMID:
            pointsSize = PRIM_SIZ_V_PYRAMID;
            indicesSize = PRIM_SIZ_I_PYRAMID;
            points = FillFloat(PRIM_ARR_V_PYRAMID, pointsSize, scale);
            indices = FillInt(PRIM_ARR_I_PYRAMID, indicesSize);
            vbo = vbo_create(points, pointsSize * sizeof(float));
            vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
            vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
            vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
            ret->drawingMode = DRAW_MODE_ELEMENTS;
            ret->renderingMode = GL_TRIANGLES;
            break;
        default:
            return NULL;
            break;
    }


    if(indicesSize >= 0) {
        IBO *ibo = ibo_create(indices, indicesSize * sizeof(ui32));
    }
    vao_add_buffer(vao, vbo);

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

static void SetCulling(ui32 cullEnable, ui32 cullFace, ui32 cullFrontFace) {
    if(cullEnable == GL_TRUE) {
        glEnable(GL_CULL_FACE);
        glCullFace(cullFace);
        glFrontFace(cullFrontFace);
    }
    else {
        glDisable(GL_CULL_FACE);
    }
}

void mesh_draw(Mesh *self) {
    mesh_draw_explicit(self, self->material);
}

void mesh_draw_explicit(Mesh *self, Material *material) {
    SetCulling(self->cullEnable, self->cullFace, self->cullFrontFace);

    material_use(material);
    vao_bind(self->model->vao);

    ui32 mode = self->drawingMode;
    ui32 vertices = self->model->vertexCount;
    ui32 indices = self->model->indicesCount;

    switch(mode) {
        case DRAW_MODE_ELEMENTS:
            glDrawElements(self->renderingMode, indices, GL_UNSIGNED_INT, NULL);
            break;
        case DRAW_MODE_ARRAYS:
        default:
            glDrawArrays(self->renderingMode, 0, vertices);
            break;
    }
};

void mesh_set_matrix(Mesh *self, mat4 matrix) {
    self->modelMatrix = (float (*)[4][4])matrix;

    // Setting the matrix, so we're going to update it as well.
    mesh_send_matrix(self, self->material->shaderProgram);
}

// Weird implementation.
void mesh_send_matrix(Mesh *self, ShaderProgram *shader) {
    shader_use(shader);
    glUniformMatrix4fv(
        glGetUniformLocation(shader->id, "u_Model"),
        1, GL_FALSE, (float*)self->modelMatrix);
}
