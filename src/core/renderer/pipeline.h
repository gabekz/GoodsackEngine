#ifndef H_POSTBUFFER
#define H_POSTBUFFER

#include <util/sysdefs.h>
#include <util/maths.h>
#include <model/material.h>

// Shadowmap
void shadowmap_init();
void shadowmap_bind();
void shadowmap_bind_texture();
Material *shadowmap_getMaterial();
vec4 *shadowmap_getMatrix();

// Final pass
void postbuffer_init(ui32 winWidth, ui32 winHeight);
void postbuffer_bind();
void postbuffer_draw();
void postbuffer_cleanup();

#endif // H_POSTBUFFER
