#include "mesh.h"

#include <asset/import/loader_obj.h>
#pragma optimize("", off)

Mesh *
mesh_assemble(const char *path, float scale)
{
    // Create the VAO
    VAO *vao = vao_create();
    vao_bind(vao);

    Mesh *mesh = malloc(sizeof(Mesh));
    mesh->vao  = vao;

    MeshData *data = load_obj(path, scale);
    mesh->meshData = data;

    VBO *vbo =
      vbo_create(data->buffers.out, data->buffers.outI * sizeof(float));
    // VBO* vbo = vbo_create(v, 24 * sizeof(float));
    // IBO* ibo = ibo_create(outIndices, (outIndicesI) * sizeof(unsigned int));

    // Push our data into our single VBO
    if (data->buffers.vL > 0) vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    if (data->buffers.vtL > 0) vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    if (data->buffers.vnL > 0) vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vbo); // VBO push -> VAO

    // TBN vertex buffer
    VBO *vboTBN = vbo_create(data->buffers.outTBN,
                             data->totalTriangles * 3 * 2 * sizeof(GLfloat));
    vbo_push(vboTBN, 3, GL_FLOAT, GL_FALSE); // tangent
    vbo_push(vboTBN, 3, GL_FLOAT, GL_FALSE); // bitangent
    vao_add_buffer(vao, vboTBN);
    // free(data->buffers.outTBN);

    return mesh;
}
