/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#include "entity/ecs.h"

#if !(USING_GENERATED_COMPONENTS)
struct ComponentLight
{
    vec4 color;
    u32 type;
};
#endif

#endif // __LIGHT_H__
