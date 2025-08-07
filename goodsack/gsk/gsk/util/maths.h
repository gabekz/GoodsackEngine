/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MATHS_H__
#define __MATHS_H__

#include <math.h>

#include "cglm/cglm.h"
#include <cglm/struct.h>

#ifdef MIN
#undef MIN
#endif // MIN

#ifdef MAX
#undef MAX
#endif // MAX

#ifdef CLAMP
#undef CLAMP
#endif // CLAMP

#define MAX(a, b)      (((a) > (b)) ? (a) : (b))
#define MIN(a, b)      (((a) < (b)) ? (a) : (b))
#define CLAMP(c, m, n) c = (MIN(MAX(c, m), n))

#endif // __MATHS_H__
