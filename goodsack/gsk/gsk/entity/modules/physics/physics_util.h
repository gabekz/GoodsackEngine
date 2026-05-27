/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_PHYSICS_UTIL_H__
#define __GSK_PHYSICS_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "entity/ecs.h"
#include "physics/physics_types.h"

gsk_PhysicsMark
gsk_physics_util_create_physics_mark(gsk_Entity entity_a, gsk_Entity entity_b);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__GSK_PHYSICS_UTIL_H__