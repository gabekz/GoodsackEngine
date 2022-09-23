#ifndef H_ECS
#define H_ECS

#include <renderer/renderer.h>
#include <util/sysdefs.h>

#define ECS_TAG_SIZE 1
#define ECS_TAG_UNUSED  0b00000000
#define ECS_TAG_USED    0b00110000

#define ECSCL_ELEMENT_SIZE(_plist) ((_plist)->component_size + ECS_TAG_SIZE)
#define ECSCL_GET(_plist, _i) ({\
        ECSComponentList *_pl = (_plist);\
        ((_pl)->components) + ((_i) * ECSCL_ELEMENT_SIZE(_pl)) + ECS_TAG_SIZE;\
    })

typedef struct _ecs ECS;
typedef struct _ecs_entity Entity;
typedef union _ecs_system ECSSystem;
typedef enum _ecs_component ECSComponent;
typedef struct _ecs_component_list ECSComponentList;

typedef void (*ECSSubscriber)(Entity);
typedef ui64 EntityId;

#define ECSEVENT_LAST ECS_UPDATE
enum ECSEvent {
    ECS_INIT = 0, ECS_DESTROY, ECS_RENDER, ECS_UPDATE
};

#define ECSCOMPONENT_LAST C_CAMERA
enum _ecs_component {
    C_CAMERA = 0,
};

/*-------------------------------------------*/

struct _ecs_entity {
    EntityId id;
    ui64 index;
    ECS *ecs;
};

/*-------------------------------------------*/

struct _ecs_component_list {
    void *components;
    ui64 component_size;
    //ui64 components_size;
    ui64 *entity_index_list;
};

struct _ecs {
    EntityId *ids, nextId;
    ui32 nextIndex;
    ui32 capacity;

    Renderer *renderer;

    ECSComponentList component_lists[ECSCOMPONENT_LAST+1];
    ECSSystem *systems;
    ui32 systems_size;

};

/*-------------------------------------------*/

union _ecs_system {
    struct {
        ECSSubscriber init, destroy, render, update;
    };

    ECSSubscriber subscribers[ECSEVENT_LAST + 1];
};

/*-------------------------------------------*/

#define _ECS_DECL_SYSTEM(_name)\
    extern void _name##_init();\
    _name##_init(ecs);

// Declare systems here
static inline void _ecs_init_internal(ECS *ecs) {
    _ECS_DECL_SYSTEM(s_camera);
}

/*-------------------------------------------*/

void _ecs_add_internal(Entity entity, ui32 component_id, void *value);

#define _ecs_add3(e, c, v) ({ __typeof__(v) _v = (v); _ecs_add_internal((e), (c), &_v); })
#define _ecs_add2(e, c) _ecs_add_internal((e), (c), NULL)

#define _ecs_add_overload(_1,_2,_3,NAME,...) NAME
#define ecs_add(...) _ecs_add_overload(__VA_ARGS__, _ecs_add3, _ecs_add2)(__VA_ARGS__)

/*-------------------------------------------*/

ECS *ecs_init(Renderer *renderer);

Entity ecs_new(ECS *self);
int ecs_has(Entity entity, ECSComponent component_id);
void *ecs_get(Entity entity, ECSComponent component_id);
void ecs_system_register(ECS *self, ECSSystem system);
void ecs_component_register(ECS *self, ui32 component_id, ui64 size);
void ecs_event(ECS *self, enum ECSEvent event);

#endif // H_ECS