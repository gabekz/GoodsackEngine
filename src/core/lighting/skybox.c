#include "lighting.h"

#include <core/api/opengl/glbuffer.h>
#include <core/shader/shader.h>
#include <model/primitives.h>

Skybox *
skybox_create(Texture *cubemap)
{

    Skybox *ret  = malloc(sizeof(Skybox));
    ret->cubemap = cubemap;

    VAO *vao = vao_create();
    ret->vao = vao;

    VBO *vbo = vbo_create(PRIM_ARR_V_CUBE, PRIM_SIZ_V_CUBE * sizeof(float));
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vbo);
    IBO *ibo =
      ibo_create(PRIM_ARR_I_CUBE, PRIM_SIZ_I_CUBE * sizeof(unsigned int));
    ibo_bind(ibo);
    free(vbo);

    ShaderProgram *shader =
      shader_create_program("../res/shaders/skybox.shader");
    ret->shader = shader;

    return ret;
}

void
skybox_draw(Skybox *self)
{

    // glDepthMask(GL_FALSE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->cubemap->id);

    shader_use(self->shader);
    vao_bind(self->vao);
    // glDrawArrays(GL_TRIANGLES, 0, 24);
    glDrawElements(GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
}
