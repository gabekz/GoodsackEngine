#include "debug_draw_line.h"

#include <core/drivers/opengl/opengl.h>
#include <tools/debug/debug_context.h>
#include <util/maths.h>

// NOTE: should take DebugContext -> contains shader information

// Line start: 0, 0, 0
// Line direction: 0, 1, 0
// Line length: 100
// Line end: 0, 100, 0

void
debug_draw_line(DebugContext *debugContext, vec3 start, vec3 end)
{
    float vertices[] = {start[0], start[1], start[2], end[0], end[1], end[2]};

    // TEST DRAW LINE
    material_use(debugContext->material);
    mat4 bbMat4 = GLM_MAT4_IDENTITY_INIT;
    glUniformMatrix4fv(glGetUniformLocation(
                         debugContext->material->shaderProgram->id, "u_Model"),
                       1,
                       GL_FALSE,
                       (float *)bbMat4);

    vao_bind(debugContext->vaoLine);
    // Update the line vertices after binding VAO
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
    glDrawArrays(GL_LINES, 0, 2);
}
