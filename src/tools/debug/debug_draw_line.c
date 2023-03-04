#include "debug_draw_line.h"

#include <core/drivers/opengl/opengl.h>
#include <util/maths.h>

// NOTE: should take DebugContext -> contains shader information

void
debug_draw_line(vec3 start, vec3 end)
{
    float vertices[] = {start[0], start[1], start[2], end[0], end[1], end[2]};

    VAO *vao = vao_create();
    VBO *vbo = vbo_create(vertices, 6 * sizeof(float));
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vbo);
}