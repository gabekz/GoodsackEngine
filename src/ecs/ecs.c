#include "ecs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <util/sysdefs.h>

ECS *ecs_init() {
    ECS *ecs = malloc(sizeof(ECS));

    // Initialize entity capacity
    ui32 capacity = 4;
    ecs->ids = malloc(capacity * sizeof(EntityId));
    ecs->capacity = capacity;
    ecs->nextId = 1;

    // Initialize systems
    ecs->systems_size = 0;
    ecs->systems = malloc(1 * sizeof(ECSSystem));
    _ecs_init_internal(ecs);

    // TODO: Declare all component-types here

    // Allocate storage for component lists
    /*
    for(int i = 0; i < ECSCOMPONENT_LAST+1; i++) {
        ui32 size = ecs->component_lists[i].component_size + ECS_TAG_SIZE;
        ecs->component_lists[i].components = calloc(capacity, size);

        printf("\n%d size", size);

        unsigned char *tag = (unsigned char *)(ecs->component_lists[i].components+size);
        *tag = ECS_TAG_USED;
        

        //printf("%01x\n", *(components+0+ECS_TAG_SIZE));
    }
    */

    return ecs;
}

Entity ecs_new(ECS *self) {
    ui32 capacity = self->capacity;
    // check if we have available capcity
    if(capacity < self->nextId) {
        capacity *= 2;
        //TODO: Reallocate component lists
    }

    Entity entity = (Entity) {
        .id  = self->nextId,
        .ecs = self
    };

    // TODO: Fill next available slot (if deletion)

    self->ids[self->nextId-1] = entity.id;
    self->nextId++; 

    // update componentlist tags
    for(int i = 0; i < ECSCOMPONENT_LAST+1; i++) {
        // TODO: mark here as "unused"
    }

    return entity;
}

#define ECSCL_ELEMENT_SIZE(_plist) ((_plist)->components_size)
#define ECSCL_GET(_plist, _i) ({\
        ECSComponentList *_pl = (_plist);\
        ((_pl)->components) + ((_i) * ECSCL_ELEMENT_SIZE(_pl));\
    })

//void _ecs_get_test() {
    // GETTING
    /*
    for(int i = 0; i < list.components_size; i++) {
    }
    */
//}

void _ecs_add_internal(Entity entity, ui32 component_id, void *value) {
    ECS *ecs = entity.ecs;
    ECSComponentList* list = &ecs->component_lists[component_id];

    // TODO: entityId should actually be it's index (for deletions)
    unsigned char *tag = (unsigned char *)(list->components+(entity.id));
    printf("old tag: %x", *tag & 0xff);
    assert(! (*tag & ECS_TAG_USED));
    *tag |= ECS_TAG_USED;
    printf("new tag: %x", *tag & 0xff);

    printf("entity ID is %d", entity.id);

    //void *component = list->components+(entity.id-1);
    if(value != NULL) {
        memcpy(list->components+(entity.id-1), value, list->component_size);
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

void ecs_component_register(ECS *self, ui32 component_id, ui64 size) {
    self->component_lists[component_id].component_size = size;

    // aligned with tag
    ui32 aSize = size + ECS_TAG_SIZE;
    self->component_lists[component_id].components = calloc(self->capacity, size);

    printf("\n%d size", aSize);

    unsigned char *tag = (unsigned char *)(self->component_lists[component_id].components+size);
    *tag = ECS_TAG_USED;
}

void ecs_event(ECS *self, enum ECSEvent event) {
    ui32 systemsCount = 1;
    ui32 entityCount = 1;

    // Loop through each system, fire the appropriate event
    for(int i = 0; i < systemsCount; i++) {
        ECSSubscriber func = self->systems[i].subscribers[event];
        if (func != NULL) {
            func();
            //continue;
        }
        // Call the system for every entity
        /*
        for (int j = 0; j < entityCount; j++) {
            f((Entity) { .id = self->ids[j], .ecs = self });
        }
        */

    }


    // TODO: determine whether or not there is a required component.
    // Go through that list instead of every entity.
}
