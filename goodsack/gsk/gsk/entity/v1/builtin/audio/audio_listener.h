/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __AUDIO_LISTENER_H__
#define __AUDIO_LISTENER_H__

#include "entity/v1/ecs.h"

#if !(USING_GENERATED_COMPONENTS)
struct ComponentAudioListener
{
    int a;
};
#endif

void
s_audio_listener_init(ECS *ecs);

#endif // __AUDIO_LISTENER_H__
