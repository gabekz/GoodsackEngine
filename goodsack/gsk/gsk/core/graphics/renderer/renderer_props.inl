/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __RENDERER_PROPS_INL__
#define __RENDERER_PROPS_INL__

#include "util/maths.h"
#include "util/sysdefs.h"

// enum Tonemappers { Reinhard = 0, ACES };

// Renderer Properties
typedef struct gsk_RendererProps
{
    u32 tonemapper;

    float exposure;
    float maxWhite;

    float gamma;
    int gammaEnable;

    u32 msaaSamples;
    int msaaEnable;

    float vignetteAmount;
    float vignetteFalloff;
    vec3 vignetteColor;

} gsk_RendererProps;

#endif // __RENDERER_PROPS_INL__