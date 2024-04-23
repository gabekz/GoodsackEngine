/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ECS_H__
#define __ECS_H__

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

#define _ecs_new_2(e, n)                     _gsk_ecs_new_internal(e, n)
#define _ecs_new_1(e)                        _gsk_ecs_new_internal(e, NULL)
#define _ecs_new_overload(_1, _2, NAME, ...) NAME
#define gsk_ecs_new(...) \
    _ecs_new_overload(__VA_ARGS__, _ecs_new_2, _ecs_new_1)(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gsk_Entity gsk_Entity;
typedef union gsk_ECSSystem gsk_ECSSystem;
typedef struct gsk_ECSComponentList gsk_ECSComponentList;

typedef void (*gsk_ECSSubscriber)(gsk_Entity);
typedef u64 gsk_EntityId;

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
    u64 index;
    gsk_ECS *ecs;
};

/*-------------------------------------------*/

struct gsk_ECSComponentList
{
    void *components;
    u64 component_size;
    u64 *entity_index_list;
};

/*-------------------------------------------*/

struct gsk_ECS
{
    gsk_EntityId *ids, nextId;
    u32 nextIndex;
    u32 capacity;

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
