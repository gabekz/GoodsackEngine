#ifndef H_ECS
#define H_ECS

#include <entity/ecsdefs.h>
#include <util/sysdefs.h>

#define _ECS_DECL_SYSTEM(_name) extern void _name##_init();

#if USING_GENERATED_COMPONENTS
#define _ECS_DECL_COMPONENT(_self, _id, _size) void()
#else
#define _ECS_DECL_COMPONENT(_self, _id, _size) \
    ecs_component_register(_self, _id, _size)
#endif

#define _ecs_add3(e, c, v)                \
    ({                                    \
        __typeof__(v) _v = (v);           \
        _ecs_add_internal((e), (c), &_v); \
    })
#define _ecs_add2(e, c) _ecs_add_internal((e), (c), NULL)

#define _ecs_add_overload(_1, _2, _3, NAME, ...) NAME
#define ecs_add(...) \
    _ecs_add_overload(__VA_ARGS__, _ecs_add3, _ecs_add2)(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ecs_entity Entity;
typedef union _ecs_system ECSSystem;
typedef struct _ecs_component_list ECSComponentList;

typedef void (*ECSSubscriber)(Entity);
typedef ui64 EntityId;

typedef struct _ecs ECS;

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

struct _ecs_entity
{
    EntityId id;
    ui64 index;
    ECS *ecs;
};

/*-------------------------------------------*/

struct _ecs_component_list
{
    void *components;
    ui64 component_size;
    // ui64 components_size;
    ui64 *entity_index_list;
};

struct _ecs
{
    EntityId *ids, nextId;
    ui32 nextIndex;
    ui32 capacity;

    Renderer *renderer;

    ECSComponentList component_lists[ECSCOMPONENT_LAST + 1];
    ECSSystem *systems;
    ui32 systems_size;
};

/*-------------------------------------------*/

union _ecs_system {
    struct
    {
        ECSSubscriber init, destroy, render, update;
    };

    ECSSubscriber subscribers[ECSEVENT_LAST + 1];
};

void
_ecs_add_internal(Entity entity, ui32 component_id, void *value);

/*-------------------------------------------*/

ECS *
ecs_init(Renderer *renderer);
Entity
ecs_new(ECS *self);

int
ecs_has(Entity entity, ECSComponentType component_id);
void *
ecs_get(Entity entity, ECSComponentType component_id);

void
ecs_system_register(ECS *self, ECSSystem system);
void
ecs_component_register(ECS *self, ui32 component_id, ui64 size);

void
ecs_event(ECS *self, enum ECSEvent event);

#ifdef __cplusplus
}
#endif

#endif // H_ECS
