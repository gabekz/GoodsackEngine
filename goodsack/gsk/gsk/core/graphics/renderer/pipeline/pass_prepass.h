/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PASS_PREPASS_H__
#define __PASS_PREPASS_H__

#include "core/graphics/material/material.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void
prepass_init();

void
prepass_bindTextures(u32 startingSlot);

void
prepass_bind();

gsk_Material *
prepass_getMaterial();

gsk_Material *
prepass_getMaterialSkinned();

u32
prepass_getPosition();
u32
prepass_getNormal();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PASS_PREPASS_H__