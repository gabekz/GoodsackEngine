/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Remove this system

#ifndef __COMPONENT_TEST_H__
#define __COMPONENT_TEST_H__

#include "entity/ecsdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !(USING_GENERATED_COMPONENTS)
typedef struct ComponentTest
{
    int movement_increment;
    float rotation_speed;
} ComponentTest;
#endif

#ifdef __cplusplus
}
#endif

#endif // __COMPONENT_TEST_H__