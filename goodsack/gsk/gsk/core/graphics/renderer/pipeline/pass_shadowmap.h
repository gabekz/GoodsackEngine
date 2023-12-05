/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PASS_SHADOWMAP_H__
#define __PASS_SHADOWMAP_H__

#include "util/maths.h"
#include "core/graphics/material/material.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct ShadowmapOptions
{
    float nearPlane, farPlane;
    float camSize;

    float normalBiasMin, normalBiasMax;
    si32 pcfSamples;
} ShadowmapOptions;

/**
 * Creates all data needed for the shadowmap
 */
void
shadowmap_init();

/**
 * Binds shadowmap framebuffer and textures
 */
void
shadowmap_bind();

/**
 * Update shadowmap projection-properties
 * @param[in] directional-light position
 */
void
shadowmap_update(vec3 lightPosition, ShadowmapOptions options);

void
shadowmap_bind_texture();
Material *
shadowmap_getMaterial();
ui32
shadowmap_getTexture();
vec4 *
shadowmap_getMatrix();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PASS_SHADOWMAP_H__
