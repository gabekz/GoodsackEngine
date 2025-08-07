/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ALSOFT_H__
#define __ALSOFT_H__

#include "util/sysdefs.h"

#include <AL/al.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int
openal_get_devices();

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
