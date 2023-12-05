/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECS_H__
#define __ECS_H__

#include "util/sysdefs.h"
#include "entity/ecsdefs.h"

#define _ECS_DECL_SYSTEM(_name) extern void _name##_init();

#if USING_GENERATED_COMPONENTS
#define _ECS_DECL_COMPONENT(_self, _id, _size) void()
#define _ECS_DECL_COMPONENT_INTERN(_self, _id, _size) \
    gsk_ecs_component_register(_self, _id, _size)
#else
#define _ECS_DECL_COMPONENT(_self, _id, _size) \
    gsk_ecs_component_register(_self, _id, _size)
#endif

#define _ecs_add3(e, c, v)                \
    ({                                    \
        __typeof__(v) _v = (v);           \
        _gsk_ecs_add_internal((e), (c), &_v); \
    })
#define _ecs_add2(e, c) _gsk_ecs_add_internal((e), (c), NULL)

#define _ecs_add_overload(_1, _2, _3, NAME, ...) NAME
#define ecs_add(...) \
    _ecs_add_overload(__VA_ARGS__, _ecs_add3, _ecs_add2)(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gsk_Entity gsk_Entity;
typedef union gsk_ECSSystem gsk_ECSSystem;
typedef struct gsk_ECSComponentList gsk_ECSComponentList;

typedef void (*gsk_ECSSubscriber)(gsk_Entity);
typedef ui64 gsk_EntityId;

typedef struct gsk_ECS gsk_ECS;

// TODO: Fix this placement (dep-cyclical issue)
#include <core/graphics/renderer/v1/renderer.h>

#if USING_GENERATED_COMPONENTS

#include <entity/__generated__/components_gen.h>

#else

#define ECSCOMPONENT_LAST C_MODEL
enum _ecs_component {
    C_TRANSFORM = 0,
    C_CAMERA,
    C_AUDIO_LISTENER,
    C_AUDIO_SOURCE,
    C_ANIMATOR,
    C_TEST,
    C_MODEL,
};
typedef enum _ecs_component ECSComponentType;

#endif

/*-------------------------------------------*/

/*
// Declare systems here
inline void _ecs_init_internal(ECS *ecs) {
    _ECS_DECL_SYSTEM(s_transform);
    _ECS_DECL_SYSTEM(s_camera);
    _ECS_DECL_SYSTEM(s_draw_mesh);
}
*/

/*-------------------------------------------*/

struct gsk_Entity
{
    gsk_EntityId id;
    ui64 index;
    gsk_ECS *ecs;
};

/*-------------------------------------------*/

struct gsk_ECSComponentList
{
    void *components;
    ui64 component_size;
    // ui64 components_size;
    ui64 *entity_index_list;
};

struct gsk_ECS
{
    gsk_EntityId *ids, nextId;
    ui32 nextIndex;
    ui32 capacity;

    gsk_Renderer *renderer;

    gsk_ECSComponentList component_lists[ECSCOMPONENT_LAST + 1];
    gsk_ECSSystem *systems;
    ui32 systems_size;
};

/*-------------------------------------------*/

union gsk_ECSSystem {
    struct
    {
        gsk_ECSSubscriber init, destroy, render, update, late_update;
    };

    gsk_ECSSubscriber subscribers[ECSEVENT_LAST + 1];
};

void
_gsk_ecs_add_internal(gsk_Entity entity, ui32 component_id, void *value);

/*-------------------------------------------*/

gsk_ECS *
gsk_ecs_init(gsk_Renderer *renderer);
gsk_Entity
gsk_ecs_new(gsk_ECS *self);

int
gsk_ecs_has(gsk_Entity entity, ECSComponentType component_id);
void *
gsk_ecs_get(gsk_Entity entity, ECSComponentType component_id);

void
gsk_ecs_system_register(gsk_ECS *self, gsk_ECSSystem system);
void
gsk_ecs_component_register(gsk_ECS *self, ui32 component_id, ui64 size);

void
gsk_ecs_event(gsk_ECS *self, enum ECSEvent event);

#ifdef __cplusplus
}
#endif

#endif // __ECS_H__
