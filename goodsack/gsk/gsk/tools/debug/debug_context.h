#ifndef H_DEBUG_CONTEXT
#define H_DEBUG_CONTEXT

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/material/material.h>

typedef struct DebugContext
{
    VAO *vaoCube;
    VAO *vaoBoundingBox;
    Material *material;

    VAO *vaoLine; // VAO for debug draw line

} DebugContext;

DebugContext *
debug_context_init();

#endif // H_DEBUG_CONTEXT