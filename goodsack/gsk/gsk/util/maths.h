/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_MATHS_H__
#define __GSK_MATHS_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <float.h>
#include <math.h>

#include "util/sysdefs.h"

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

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
//#define CLAMP(c, m, n) c = (MIN(MAX(c, m), n))
#define CLAMP(x, low, high) \
    (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifndef NEAR_ZERO
#define NEAR_ZERO 1e-6f
#endif // NEAR_ZERO

#ifndef PI
#define PI 3.14159f
#endif // PI

#define IS_NEAR_ZERO(a) ((fabsf(a) <= NEAR_ZERO) ? TRUE : FALSE)

#define FDIV_SAFE(a, b) ((b == 0) ? 0 : a / b)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_MATHS_H__
