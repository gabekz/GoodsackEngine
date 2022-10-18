#ifndef H_PASS_SHADOWMAP
#define H_PASS_SHADOWMAP

#include <util/maths.h>
#include <model/material.h>

void shadowmap_init();
void shadowmap_bind();
void shadowmap_bind_texture();
Material *shadowmap_getMaterial();
vec4 *shadowmap_getMatrix();

#endif
