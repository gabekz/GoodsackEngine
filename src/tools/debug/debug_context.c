#include "debug_context.h"

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

        ret->material = material_create(NULL, "../res/shaders/white.shader", 0);
    }

    return ret;
}
