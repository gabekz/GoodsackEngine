#include "ecs.h"
#include <stdlib.h>
#include <stdio.h>

#include <util/sysdefs.h>

ECS *ecs_init() {
    ECS *ret = malloc(sizeof(ECS));

    ui32 capacity = 64;
    ret->ids = malloc(capacity * sizeof(EntityId));
    ret->capacity = capacity;
    ret->nextId = 1;

    ret->systems_size = 0;
    ret->systems = malloc(1 * sizeof(ECSSystem));

    ret->components_size = 0;
    ret->components = malloc(1 * sizeof(ECSComponent));

    // Decalre ECS Systems
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

void _ecs_add_internal(Entity entity, ui32 component_id, void *value) {

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
