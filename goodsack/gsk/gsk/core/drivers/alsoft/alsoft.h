/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ALSOFT_H__
#define __ALSOFT_H__

#include "util/sysdefs.h"

#include "alsoft_buffer.h"
#include "alsoft_debug.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

ALCdevice *
openal_get_device();

int
openal_init();

void
openal_cleanup();

ALuint
openal_generate_source();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ALSOFT_H__
