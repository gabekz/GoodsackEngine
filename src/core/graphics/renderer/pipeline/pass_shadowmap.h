#ifndef H_PASS_SHADOWMAP
#define H_PASS_SHADOWMAP

#include <core/graphics/material/material.h>
#include <util/maths.h>

void
shadowmap_init();
void
shadowmap_bind();
void
shadowmap_bind_texture();
Material *
shadowmap_getMaterial();
vec4 *
shadowmap_getMatrix();

#endif
