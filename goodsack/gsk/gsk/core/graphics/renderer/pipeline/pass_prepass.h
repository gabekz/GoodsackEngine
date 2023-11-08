#ifndef H_PASS_PREPASS
#define H_PASS_PREPASS

#include <core/graphics/material/material.h>

void
prepass_init();

void
prepass_bindTextures(ui32 startingSlot);

void
prepass_bind();

Material *
prepass_getMaterial();

ui32
prepass_getPosition();
ui32
prepass_getNormal();

#endif // H_PASS_PREPASS