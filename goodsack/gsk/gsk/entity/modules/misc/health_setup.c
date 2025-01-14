/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "health_setup.h"

#define DEFAULT_MAX_HEALTH 100

static void
init(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_HEALTH))) return;
    struct ComponentHealth *cmp_health = gsk_ecs_get(entity, C_HEALTH);

    if (!cmp_health->max_health)
    {
        cmp_health->max_health == DEFAULT_MAX_HEALTH;
    }

    if (!cmp_health->current_health)
    {
        cmp_health->current_health = cmp_health->max_health;
    }

    cmp_health->last_health = cmp_health->current_health;
    cmp_health->is_alive    = (cmp_health->current_health > 0) ? TRUE : FALSE;
}

void
s_health_setup_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init = (gsk_ECSSubscriber)init,
                            }));
}
