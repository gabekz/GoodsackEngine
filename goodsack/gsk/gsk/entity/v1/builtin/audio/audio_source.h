/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __AUDIO_SOURCE_H__
#define __AUDIO_SOURCE_H__

#include "entity/v1/ecs.h"
#include "core/drivers/alsoft/alsoft.h"

#if !(USING_GENERATED_COMPONENTS)
typedef struct ComponentAudioSource
{
    ALuint bufferId;
    const char *filePath;

    int looping;
} ComponentAudioSource;
#endif

void
s_audio_source_init(gsk_ECS *ecs);

#endif // __AUDIO_SOURCE_H__
