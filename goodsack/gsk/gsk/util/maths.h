/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MATHS_H__
#define __MATHS_H__

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#ifdef CLAMP
#undef CLAMP
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define CLAMP(c, m, n) c = (MIN(MAX(c, m), n))

#ifdef __cplusplus
extern "C" {
#endif
#include "cglm/cglm.h"
#include <cglm/struct.h>
#ifdef __cplusplus
}
#endif
#include "math.h"

#endif // __MATHS_H__
