/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "mod_camera.h"

#include "entity/ecs.h"
#include "util/array_list.h"
#include "util/logger.h"
#include "util/sysdefs.h"

void
gsk_mod_camera_shake_add(gsk_Entity entity_camera,
                         f32 amount,
                         f32 speed,
                         f32 jitter)
{
    if (!(gsk_ecs_has(entity_camera, C_CAMERA))) { return; }
    gsk_C_Camera *cmp_camera = gsk_ecs_get(entity_camera, C_CAMERA);

    ArrayList *p_list = cmp_camera->list_shakers;

    if (p_list->list_next >= GSK_MOD_CAMERA_MAX_SHAKERS)
    {
        LOG_WARN("cannot add shaker to camera - maximum shakers reached!");
        return;
    }

    gsk_mod_CameraShaker shaker = {
      .is_running   = TRUE,
      .shake_amount = amount,
      .shake_speed  = speed,
      .shake_jitter = jitter,
      .shake_scalar = 1.0f,
    };

    // check if we want to override an existing non-active shaker
    for (int i = 0; i < p_list->list_next; i++)
    {
        gsk_mod_CameraShaker *p_shaker = LIST_GET(p_list, i);
        if (p_shaker->is_running == FALSE)
        {
            *p_shaker = shaker;
            return;
        }
    }

    LIST_PUSH(p_list, &shaker);
}