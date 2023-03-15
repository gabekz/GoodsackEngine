#ifndef H_PASS_SCREEN
#define H_PASS_SCREEN

#include <core/graphics/renderer/renderer_props.inl>
#include <util/sysdefs.h>

void
postbuffer_init(ui32 width, ui32 height);
void
postbuffer_bind(int enableMSAA);
void
postbuffer_draw(RendererProps *properties);
void
postbuffer_resize(ui32 winWidth, ui32 winHeight);
void
postbuffer_cleanup();

#endif
