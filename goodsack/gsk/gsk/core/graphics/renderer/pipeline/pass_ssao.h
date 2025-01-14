/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PASS_SSAO_H__
#define __PASS_SSAO_H__

#include "util/sysdefs.h"

typedef struct SsaoOptions
{
    float strength, bias, radius;
    int kernelSize;

} SsaoOptions;

void
pass_ssao_init();

void
pass_ssao_bind();

u32
pass_ssao_getOutputTextureId();

#endif // __PASS_SSAO_H__