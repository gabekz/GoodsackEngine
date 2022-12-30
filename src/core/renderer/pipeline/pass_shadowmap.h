#ifndef H_PASS_SHADOWMAP
#define H_PASS_SHADOWMAP

#include <model/material.h>
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
