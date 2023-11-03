#include "debug_context.h"

#include <util/filesystem.h>

#include <core/device/device.h>
#include <core/drivers/opengl/opengl.h>

#include <core/graphics/material/material.h>
#include <core/graphics/mesh/primitives.h>

DebugContext *
debug_context_init()
{
    DebugContext *ret = malloc(sizeof(DebugContext));

    if (DEVICE_API_OPENGL) {
        ret->vaoCube = vao_create();

        float *vertices = PRIM_ARR_V_CUBE;
        for (int i = 0; i < PRIM_SIZ_V_CUBE; i++) {
            vertices[i] *= 0.02f;
        }

        VBO *vboCube = vbo_create(vertices, PRIM_SIZ_V_CUBE * sizeof(float));
        vbo_push(vboCube, 3, GL_FLOAT, GL_FALSE);
        vao_add_buffer(ret->vaoCube, vboCube);

        IBO *ibo = ibo_create(PRIM_ARR_I_CUBE, PRIM_SIZ_I_CUBE * sizeof(ui32));
        ibo_bind(ibo);

        ret->material =
          material_create(NULL, GSK_PATH("gsk://shaders/white.shader"), 0);

        // Bounding box
        ret->vaoBoundingBox = vao_create();
        vao_bind(ret->vaoBoundingBox);
        VBO *vboBoundingBox =
          vbo_create(PRIM_ARR_V_CUBE2, PRIM_SIZ_V_CUBE2 * sizeof(float));
        vbo_bind(vboBoundingBox);
        vbo_push(vboBoundingBox, 4, GL_FLOAT, GL_FALSE);
        vao_add_buffer(ret->vaoBoundingBox, vboBoundingBox);

        IBO *iboBoundingBox =
          ibo_create(PRIM_ARR_I_CUBE2, PRIM_SIZ_I_CUBE2 * sizeof(unsigned int));
        ibo_bind(iboBoundingBox);

        // Line VAO
        vec3 lineStart    = GLM_VEC3_ZERO_INIT;
        vec3 lineEnd      = GLM_VEC3_ZERO_INIT;
        float lineverts[] = {lineStart[0],
                             lineStart[1],
                             lineStart[2],
                             lineEnd[0],
                             lineEnd[1],
                             lineEnd[2]};
        ret->vaoLine      = vao_create();
        VBO *lineVbo      = vbo_create(lineverts, 6 * sizeof(float));
        vbo_push(lineVbo, 3, GL_FLOAT, GL_FALSE);
        vao_add_buffer(ret->vaoLine, lineVbo);

        // OpenGL Line smoothing
        glLineWidth(1.0f);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    }

    return ret;
}
