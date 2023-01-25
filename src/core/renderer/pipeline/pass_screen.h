#ifndef H_PASS_SCREEN
#define H_PASS_SCREEN

#include <util/sysdefs.h>

void
postbuffer_init(ui32 winWidth, ui32 winHeight);
void
postbuffer_bind(int enableMSAA);
void
postbuffer_draw(ui32 winWidth, ui32 winHeight, int enableMSAA, ui32 tonemapper, float exposure, float maxWhite, float gamma, int gammaEnable);
void
postbuffer_resize(ui32 winWidth, ui32 winHeight);
void
postbuffer_cleanup();

#endif
