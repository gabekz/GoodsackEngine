#include "mesh.h"
#include <stdlib.h>

#include <loaders/loader_obj.h>
#include "primitives.h"
#include "gfx.h"

Mesh *mesh_create(const char *modelPath, ui32 shaderId) {

// Initialize mesh object 
    Mesh *ret = malloc(sizeof(Mesh));
    ret->shaderId = shaderId;

    VAO *vao = load_obj(modelPath); // NOTE: load_obj() keeps the VAO bound 
    ret->vao = vao;

    return ret;
}

void mesh_draw(Mesh *self) {
    glUseProgram(self->shaderId);
    vao_bind(self->vao);
    glDrawArrays(GL_TRIANGLES, 0, 23232);
}
