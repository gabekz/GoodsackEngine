#ifndef H_PASS_SCREEN
#define H_PASS_SCREEN

#include <util/sysdefs.h>

void postbuffer_init(ui32 winWidth, ui32 winHeight);
void postbuffer_bind();
void postbuffer_draw(ui32 winWidth, ui32 winHeight);
void postbuffer_cleanup();

#endif
