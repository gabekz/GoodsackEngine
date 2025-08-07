/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ALSOFT_DEBUG_H__
#define __ALSOFT_DEBUG_H__

#include "util/sysdefs.h"

#include <AL/al.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if SYS_DEBUG == SYS_ENABLED
#define AL_CHECK(x)              \
    do                           \
    {                            \
        x;                       \
        openal_debug_callback(); \
    } while (0)
#else
#define AL_CHECK(x) x;
#endif

int
openal_debug_callback();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ALSOFT_DEBUG_H__
