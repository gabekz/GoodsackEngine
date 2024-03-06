/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MOD_PHYSICS_H__
#define __MOD_PHYSICS_H__

#include "util/sysdefs.h"

#include "entity/ecs.h"
#include "physics/physics_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_mod_RaycastResult
{
    gsk_Entity entity;
    u8 has_collision;

} gsk_mod_RaycastResult;

gsk_mod_RaycastResult
gsk_mod_physics_raycast(gsk_Entity entity_caller, gsk_Raycast *raycast);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MOD_PHYSICS_H__