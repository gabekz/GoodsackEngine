#ifndef H_DEBUG_DRAW_BOUNDS
#define H_DEBUG_DRAW_BOUNDS

#include <core/graphics/mesh/mesh.h>
#include <tools/debug/debug_context.h>

void
debug_draw_bounds(DebugContext *debugContext,
                  vec3 corners[2],
                  mat4 modelMatrix);

#endif // H_DEBUG_DRAW_BOUNDS