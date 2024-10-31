/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECSDEFS_H__
#define __ECSDEFS_H__

enum ECSEvent {
    ECS_INIT = 0,
    ECS_DESTROY,
    ECS_RENDER,
    ECS_ON_COLLIDE,
    ECS_FIXED_UPDATE,
    ECS_UPDATE,
    ECS_LATE_UPDATE
};

#define ECSEVENT_FIRST ECS_INIT
#define ECSEVENT_LAST  ECS_LATE_UPDATE

#define ECSEVENT_STRING(x) (_gsk_ecs_EventToString(x))

#define ECSEVENT_CFN_NAMES \
    init, destroy, render, on_collide, fixed_update, update, late_update

#define ECS_TAG_SIZE   1 // Tag size as bytes
#define ECS_TAG_UNUSED 0b00000000
#define ECS_TAG_USED   0b00110000

#define ECS_ENT_FLAG_INITIALIZED 0x01
#define ECS_ENT_FLAG_PENDING     0x00

#define ECS_VAL_NAN      0 // TODO: could be disabled
#define ECS_VAL_ENABLED  1
#define ECS_VAL_DISABLED 2

#define ECS_FIRST_ID 300

#define ECS_ENT_CAPACITY 11
#define ECS_NAME_LEN_MAX 128

// Should always be True
#define USING_GENERATED_COMPONENTS 1

// Component struct Packing/Padding
#define ECS_COMPONENTS_PACKED 0
#if defined(SYS_ENV_WIN)
#define ECS_COMPONENTS_ALIGN_BYTES 16
#else
#define ECS_COMPONENTS_ALIGN_BYTES 16
#endif // defined(SYS_ENV_WIN)

#ifdef __cplusplus
extern "C" {
#endif // extern "C"

// Event enum as a [Lua] function name
inline const char *
_gsk_ecs_EventToString(int event)
{
    switch (event)
    {
    case ECS_INIT: return "start";
    case ECS_DESTROY: return "destroy";
    case ECS_RENDER: return "render";
    case ECS_ON_COLLIDE: return "on_collide";
    case ECS_FIXED_UPDATE: return "fixed_update";
    case ECS_UPDATE: return "update";
    case ECS_LATE_UPDATE: return "late_update";
    default: return "";
    }
}

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef ECS_SYSTEM
// #define ECS_SYSTEM_DECLARE(_name) static
#endif

#endif // H_ECSDEFS
