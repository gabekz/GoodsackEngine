/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LIGHTING_H__
#define __LIGHTING_H__

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

// TODO: Move to thirdparty directive - gkutuzov/GoodsackEngine#19
#include <cglm/cglm.h>
#include <cglm/struct.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum lightType_t { Directional = 0, Point = 1, Spot = 2 } LightType;

typedef struct light_t
{
    vec3 position;
    vec4 color;
    LightType type;
    float strength;

    ui32 ubo;
} Light;

Light *
lighting_initialize(vec3 lightPos, vec4 lightColor);
void
lighting_update(Light *light, vec3 lightPos, vec4 lightColor);

#ifdef __cplusplus
}
#endif

#endif // __LIGHTING_H__