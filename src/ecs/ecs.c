#include "ecs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <util/sysdefs.h>

ECS *ecs_init() {
    ECS *ret = malloc(sizeof(ECS));

    // Initialize entity capacity
    ui32 capacity = 64;
    ret->ids = malloc(capacity * sizeof(EntityId));
    ret->capacity = capacity;
    ret->nextId = 1;

    ret->systems_size = 0;
    ret->systems = malloc(1 * sizeof(ECSSystem));

    // clear every component list
    for(int i = 0; i < ECSCOMPONENT_LAST+1; i++) {
        ret->component_lists[i].components_size = 0;
    }

    // Decalare ECS Systems
    _ecs_init_internal(ret);

    return ret;
}

Entity ecs_new(ECS *self) {
    ui32 capacity = self->capacity;

    Entity entity = (Entity) {
        .id  = self->nextId,
        .ecs = self
    };

    // check if we have available capcity
    if(capacity < self->nextId) {
        // allocate more storage
        capacity *= 2;
        //self->ids = realloc();

    }
    self->ids[self->nextId-1] = entity.id;
    self->nextId++; 


    return entity;

}

#define ECSCL_ELEMENT_SIZE(_plist) ((_plist)->components_size)
#define ECSCL_GET(_plist, _i) ({\
        ECSComponentList *_pl = (_plist);\
        ((_pl)->components) + ((_i) * ECSCL_ELEMENT_SIZE(_pl));\
    })

void _ecs_get_test() {
    // GETTING
    /*
    for(int i = 0; i < list.components_size; i++) {
    }
    */
}

void _ecs_add_internal(Entity entity, ui32 component_id, void *value) {
    ECS *ecs = entity.ecs;
    ECSComponentList* list = &ecs->component_lists[component_id];

    list->components_size += 1;
    list->entity_index_list[list->components_size] = entity.id;

    void *component = list->components+list->components_size-1;

    if(value != NULL) {
        memcpy(component, value, list->components_size);
        //list = realloc(list, list.components_size+1 * sizeof());
    }
}


void *ecs_get(Entity entity, ECSComponent component_id) {
    //assert(ecs_has(entity, component));
    //return ECSCL_GET(&entity.ecs->component_lists[component_id], entity.id);
}

void ecs_system_register(ECS *self, ECSSystem system) {

    ui32 newSize = self->systems_size+1;

    ECSSystem* p = realloc(self->systems, newSize * sizeof(ECSSystem));
    self->systems = p;

    self->systems[newSize-1] = system;
}

void ecs_event(ECS *self, enum ECSEvent event) {
    ui32 systemsCount = 1;
    ui32 entityCount = 1;

    ECSSystem *list;

    // Loop through each system, fire the appropriate event
    for(int i = 0; i < systemsCount; i++) {
        ECSSubscriber f = self->systems[i].subscribers[event];
        if (f == NULL) {
            continue;
        }
        // Call the system for every entity
        for (int j = 0; j < entityCount; j++) {
            f((Entity) { .id = self->ids[j], .ecs = self });
        }

    }


    // TODO: determine whether or not there is a required component.
    // Go through that list instead of every entity.
}
