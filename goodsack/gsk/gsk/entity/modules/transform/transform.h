/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "entity/ecs.h"
#include "util/maths.h"

#if !(USING_GENERATED_COMPONENTS)
struct ComponentTransform
{
    vec3 position, orientation, scale;
    struct
    {
        mat4 model;
    } mvp;
    float test;
};
#endif

void
transform_position(struct ComponentTransform *transform, vec3 position);
void
transform_translate(struct ComponentTransform *transform, vec3 position);
void
transform_rotate(struct ComponentTransform *transform, vec3 rotation);
void
transform_scale(struct ComponentTransform *transform);

void
s_transform_init(gsk_ECS *ecs);

#endif // H_C_TRANSFORM
