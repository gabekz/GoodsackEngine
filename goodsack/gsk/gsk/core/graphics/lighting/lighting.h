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

#define MAX_LIGHTS 4

typedef enum LightType { Directional = 0, Point = 1, Spot = 2 } LightType;

typedef struct gsk_Light
{
    vec3 position;
    vec4 color;
    LightType type;
    float strength;
} gsk_Light;

typedef struct gsk_LightingData
{
    u32 ubo_id, ubo_size;
    u32 total_lights;
    gsk_Light lights[MAX_LIGHTS];
} gsk_LightingData;

gsk_LightingData
gsk_lighting_initialize(u32 ubo_binding);

void
gsk_lighting_add_light(gsk_LightingData *p_lighting_data,
                       vec3 light_position,
                       vec4 light_color);

#if 0
void
gsk_lighting_update(gsk_Light *light, vec3 lightPos, vec4 lightColor);
#endif

void
gsk_lighting_update(gsk_LightingData *p_lighting_data);

#ifdef __cplusplus
}
#endif

#endif // __LIGHTING_H__