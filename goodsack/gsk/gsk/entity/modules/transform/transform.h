/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "entity/ecs.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#define _TRANSFORM_QUATERNION TRUE

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
transform_rotate(struct ComponentTransform *transform, vec3 rotation);

void
transform_set_rotation(struct ComponentTransform *transform, versor quat);

void
transform_set_rotation_xyz(struct ComponentTransform *transform, vec3 rotation);

void
s_transform_init(gsk_ECS *ecs);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // H_C_TRANSFORM
