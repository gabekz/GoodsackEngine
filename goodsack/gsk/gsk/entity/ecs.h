/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECS_H__
#define __ECS_H__

#include <stdio.h>

#include "entity/ecsdefs.h"
#include "util/sysdefs.h"

#define _ECS_DECL_SYSTEM(_name) extern void _name##_init();

#if USING_GENERATED_COMPONENTS
#define _ECS_DECL_COMPONENT(_self, _id, _size) void()
#define _ECS_DECL_COMPONENT_INTERN(_self, _id, _size) \
    gsk_ecs_component_register(_self, _id, _size)
#else
#define _ECS_DECL_COMPONENT(_self, _id, _size) \
    gsk_ecs_component_register(_self, _id, _size)
#endif

#define _ecs_add3(e, c, v)                    \
    ({                                        \
        __typeof__(v) _v = (v);               \
        _gsk_ecs_add_internal((e), (c), &_v); \
    })
#define _ecs_add2(e, c) _gsk_ecs_add_internal((e), (c), NULL)

#define _ecs_add_overload(_1, _2, _3, NAME, ...) NAME
#define ecs_add(...) \
    _ecs_add_overload(__VA_ARGS__, _ecs_add3, _ecs_add2)(__VA_ARGS__)

#define _ecs_new_2(e, n) _gsk_ecs_new_internal(e, n)
#define _ecs_new_1(e)    _gsk_ecs_new_internal(e, NULL)

#define _ecs_new_overload(_1, _2, NAME, ...) NAME
#define gsk_ecs_new(...) \
    _EXPAND(_ecs_new_overload(__VA_ARGS__, _ecs_new_2, _ecs_new_1)(__VA_ARGS__))

//#define _ecs_new_overload(_1, _2, NAME, ...) NAME
//#define gsk_ecs_new(...) \
//    _ecs_new_overload(__VA_ARGS__, _ecs_new_2, _ecs_new_1)(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gsk_Entity gsk_Entity;
typedef union gsk_ECSSystem gsk_ECSSystem;
typedef struct gsk_ECSComponentList gsk_ECSComponentList;

typedef u64 gsk_EntityId;
typedef s32 gsk_EntityFlags;
typedef s32 gsk_EntityLayer;

typedef struct gsk_ECS gsk_ECS;

typedef void (*gsk_ECSSubscriber)(gsk_Entity);

// TODO: Fix this placement (dep-cyclical issue)
#include <core/graphics/renderer/v1/renderer.h>

#include <entity/__generated__/components_gen.h>

struct gsk_Entity
{
    gsk_EntityId id;
    u64 index;
    gsk_ECS *ecs;
};

/*-------------------------------------------*/

struct gsk_ECSComponentList
{
    void *components;
    u64 component_size;
};

/*-------------------------------------------*/

struct gsk_ECS
{
    u32 nextIndex;
    u32 capacity;

    gsk_EntityId *p_ent_ids, nextId;
    gsk_EntityFlags *p_ent_flags;
    gsk_EntityLayer *p_ent_layers;

    char **entity_names;

    gsk_Renderer *renderer;

    gsk_ECSComponentList component_lists[ECSCOMPONENT_LAST + 1];
    gsk_ECSSystem *systems;
    u32 systems_size;
};

/*-------------------------------------------*/

union gsk_ECSSystem {
    struct
    {
        gsk_ECSSubscriber ECSEVENT_CFN_NAMES;
    };

    gsk_ECSSubscriber subscribers[ECSEVENT_LAST + 1];
};

/*-------------------------------------------*/

void
_gsk_ecs_add_internal(gsk_Entity entity, u32 component_id, void *value);

s32
_gsk_ecs_set_internal(gsk_Entity entity, u32 component_id, u8 is_active);

/*-------------------------------------------*/

gsk_Entity
gsk_ecs_ent(gsk_ECS *self, gsk_EntityId id);

gsk_ECS *
gsk_ecs_init(gsk_Renderer *renderer);

gsk_Entity
_gsk_ecs_new_internal(gsk_ECS *self, char *name);

void
gsk_ecs_ent_set_active(gsk_Entity entity, u8 is_active);

void
gsk_ecs_ent_set_layer(gsk_Entity entity, gsk_EntityLayer layer);

void
gsk_ecs_ent_destroy(gsk_Entity entity);

int
gsk_ecs_has(gsk_Entity entity, ECSComponentType component_id);

void *
gsk_ecs_get(gsk_Entity entity, ECSComponentType component_id);

void
gsk_ecs_system_register(gsk_ECS *self, gsk_ECSSystem system);

void
gsk_ecs_component_register(gsk_ECS *self, u32 component_id, u64 size);

void
gsk_ecs_event(gsk_ECS *self, enum ECSEvent event);

#ifdef __cplusplus
}
#endif

#endif // __ECS_H__
