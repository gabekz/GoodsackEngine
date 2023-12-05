/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PASS_PREPASS_H__
#define __PASS_PREPASS_H__

#include "core/graphics/material/material.h"

void
prepass_init();

void
prepass_bindTextures(u32 startingSlot);

void
prepass_bind();

gsk_Material *
prepass_getMaterial();

u32
prepass_getPosition();
u32
prepass_getNormal();

#endif // __PASS_PREPASS_H__