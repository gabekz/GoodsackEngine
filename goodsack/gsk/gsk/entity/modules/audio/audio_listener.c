/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "audio_listener.h"

#include "core/drivers/alsoft/alsoft.h"
#include "core/drivers/alsoft/alsoft_debug.h"

#include "entity/modules/transform/transform.h"

// TODO: Move to thirdparty directive - gabekz/GoodsackEngine#19
#include <AL/al.h>

static void
update(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_AUDIO_LISTENER))) return;

    // AL_CHECK(alListener3f(AL_POSITION, 0, 0, 1.0f));
    // AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));

    if ((gsk_ecs_has(e, C_TRANSFORM)))
    {
        struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
        AL_CHECK(alListener3f(AL_POSITION,
                              transform->position[0],
                              transform->position[1],
                              transform->position[2]));
        AL_CHECK(alListener3f(AL_VELOCITY, 0, 0, 0));

        if ((gsk_ecs_has(e, C_CAMERA)))
        {
            struct ComponentCamera *camera = gsk_ecs_get(e, C_CAMERA);
            ALfloat listenerOrientation[]  = {
              // View
              camera->front[0],
              0.0f,
              camera->front[2],
              // Axis-Up
              camera->axisUp[0],
              camera->axisUp[1],
              camera->axisUp[2],
            };
            AL_CHECK(alListenerfv(AL_ORIENTATION, listenerOrientation));
        }
    }
}

void
s_audio_listener_init(gsk_ECS *ecs)
{
    //_ECS_DECL_COMPONENT(
    // ecs, C_AUDIO_LISTENER, sizeof(struct ComponentAudioListener));
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .update = (gsk_ECSSubscriber)update,
                            }));
}
