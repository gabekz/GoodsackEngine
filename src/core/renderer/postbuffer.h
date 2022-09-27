#ifndef H_POSTBUFFER
#define H_POSTBUFFER

#include <util/sysdefs.h>

void postbuffer_init(ui32 winWidth, ui32 winHeight);
void postbuffer_bind();
void postbuffer_draw();
void postbuffer_cleanup();

#endif // H_POSTBUFFER
