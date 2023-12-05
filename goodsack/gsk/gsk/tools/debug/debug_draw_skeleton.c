/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_draw_skeleton.h"

#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/primitives.h"
#include "tools/debug/debug_context.h"

void
debug_draw_skeleton(DebugContext *debugContext, gsk_Skeleton *skeleton)
{
    vao_bind(debugContext->vaoCube);
    gsk_material_use(debugContext->material);

    for (int i = 0; i < skeleton->jointsCount; i++) {
        glUniformMatrix4fv(
          glGetUniformLocation(debugContext->material->shaderProgram->id,
                               "u_Model"),
          1,
          GL_FALSE,
          (float *)skeleton->joints[i]->pose.mTransform);

        glDrawElements(
          GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
    }
}