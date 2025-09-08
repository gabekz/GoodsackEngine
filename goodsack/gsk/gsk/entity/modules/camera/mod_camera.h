/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_MOD_CAMERA_H__
#define __GSK_MOD_CAMERA_H__

#include "entity/ecs.h"
#include "util/sysdefs.h"

#define GSK_MOD_CAMERA_MAX_SHAKERS    16
#define GSK_MOD_CAMERA_SHAKER_INVALID -1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_mod_CameraShaker
{
    u8 is_running;
    f32 shake_amount;
    f32 shake_speed;
    f32 shake_jitter;
    f32 shake_scalar;
} gsk_mod_CameraShaker;

s32
gsk_mod_camera_shake_add(gsk_Entity entity_camera,
                         f32 amount,
                         f32 speed,
                         f32 jitter);

gsk_mod_CameraShaker *
gsk_mod_camera_shake_get(gsk_Entity entity_camera, s32 shaker_index);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_MOD_CAMERA_H__