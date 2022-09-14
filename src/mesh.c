#include "mesh.h"
#include <stdlib.h>
#include <stdarg.h>

#include <loaders/loader_obj.h>
#include "primitives.h"
#include "gfx.h"

Material *material_create(ShaderProgram *shader, ui32 textureCount, ...) {
    Material *ret = malloc(sizeof(Material));

    va_list ap;
    va_start(ap, textureCount);
    va_end(ap);

    //Texture *textures[textureCount]; 
    //for(int i = 0; i < textureCount; i++) {
        // TODO: reallocate here
    //    textures[i] = va_arg(ap, Texture*);
    //}

    //ret->textures = textures;
    ret->texturesCount = textureCount;
    ret->shaderProgram = shader;

    return ret;
}

void material_use(Material *self) {
    if(self->texturesCount > 0) {
        for(int i = 0; i < self->texturesCount; i++) {
            texture_bind(self->textures[i], i);
        }
    }
    shader_use(self->shaderProgram);
}

void material_uniform(Material *self, char *value, ui32 type, void *data) {
    shader_use(self->shaderProgram);
    ui32 location = glGetUniformLocation(self->shaderProgram->id, value);

    glUniformMatrix4fv(location, 1, GL_FALSE, data);
}

Mesh *mesh_create_obj(Material *material, const char* modelPath) {
    Mesh *ret = malloc(sizeof(Mesh));
    VAO *vao = load_obj(modelPath);

    ret->vertexCount = 2550; // TODO: pull from OBJ

    ret->material = material;
    ret->vao = vao;
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
