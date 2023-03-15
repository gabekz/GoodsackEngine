#include "ecs.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/sysdefs.h>

#include <ecs/builtin/camera/camera.h>
#include <ecs/builtin/model/model_draw.h>
#include <ecs/builtin/transform/transform.h>

#include <ecs/builtin/audio/audio_listener.h>
#include <ecs/builtin/audio/audio_source.h>

#include <ecs/builtin/animator/animator.h>

ECS *
ecs_init(Renderer *renderer)
{
    ECS *ecs = malloc(sizeof(ECS));

    // Initialize entity capacity
    ui32 capacity  = 64;
    ecs->ids       = malloc(capacity * sizeof(EntityId));
    ecs->capacity  = capacity;
    ecs->nextId    = 1;
    ecs->nextIndex = 0;

    ecs->renderer = renderer;

    // Initialize systems and components
    ecs->systems_size = 0;
    ecs->systems      = malloc(1 * sizeof(ECSSystem));

    s_transform_init(ecs);
    s_camera_init(ecs);
    s_model_draw_init(ecs);
    s_audio_listener_init(ecs);
    s_audio_source_init(ecs);
    s_animator_init(ecs);

    //_ecs_init_internal(ecs);

    return ecs;
}

Entity
ecs_new(ECS *self)
{
    ui32 capacity = self->capacity;
    // check if we have available capcity
    if (capacity < self->nextId) {
        capacity *= 2;
        // TODO: Reallocate component lists
    }

    Entity entity =
      (Entity) {.id    = self->nextId,
                .index = self->nextIndex, // TODO: alignment (for deletion)
                .ecs   = self};

    // TODO: Fill next available slot (if deletion)

    self->ids[self->nextId - 1] = entity.id;
    self->nextId++;
    self->nextIndex++;

    return entity;
}

void
_ecs_add_internal(Entity entity, ui32 component_id, void *value)
{
    ECS *ecs               = entity.ecs;
    ECSComponentList *list = &ecs->component_lists[component_id];
    ui32 size              = (entity.index * ECS_TAG_SIZE) +
                (list->component_size * (entity.index + 1));
    // printf("index of tag for Entity [%d]: %d", entity.index, size);

    char *tag = (char *)((void *)list->components) + size;
    *tag |= ECS_TAG_USED;
    // printf("old tag: %x", *tag & 0xff);
    // assert(! (*tag & ECS_TAG_USED));
    //*tag |= ECS_TAG_USED;
    // printf("new tag: %x", *tag & 0xff);

    // printf("entity ID is %d", (int)entity.id);

    // void *component = list->components+(entity.id-1);
    ui32 index =
      (entity.index * ECS_TAG_SIZE) + (list->component_size * (entity.index));
    if (value != NULL) {
        memcpy((char *)(((ECSComponentList *)list->components) + index),
               value,
               list->component_size);
        // list = realloc(list, list.components_size+1 * sizeof());
    }
}

int
ecs_has(Entity entity, ECSComponent component_id)
{
    ECSComponentList *list = &entity.ecs->component_lists[component_id];

    ui32 size = (entity.index * ECS_TAG_SIZE) +
                (list->component_size * (entity.index + 1));
    // printf("index of tag for Entity [%d]: %d", entity.index, size);
    char *tag = (char *)((void *)list->components) + size;
    int value = *tag;
    // if(value > 0 && entity.index == 1)
    // printf("\necs_has - component: %d, index %d, entity index: %d\n",
    // component_id, size, entity.index);
    // return value;

    return (value > 0) ? 1 : 0;
}

void *
ecs_get(Entity entity, ECSComponent component_id)
{
    assert(ecs_has(entity, component_id));
    ECSComponentList *list = &entity.ecs->component_lists[component_id];

    ui32 size =
      ((entity.index * ECS_TAG_SIZE) + (list->component_size * (entity.index)));
    // size = size - (list->component_size - 1);
    //(list->component_size * (entity.index));
    // printf("\necs_get - id: %d, index %d, id: %d", component_id, size,
    // entity.id);
    return (
      char *)((ECSComponentList *)(entity.ecs->component_lists[component_id]
                                     .components) +
              size);
    // return ECSCL_GET(&entity.ecs->component_lists[component_id], entity.id);
}

void
ecs_system_register(ECS *self, ECSSystem system)
{
    ui32 newSize = self->systems_size + 1;

    ECSSystem *p  = realloc(self->systems, newSize * sizeof(ECSSystem));
    self->systems = p;

    self->systems[newSize - 1] = system;
    self->systems_size         = newSize;
}

void
ecs_component_register(ECS *self, ui32 component_id, ui64 size)
{
    self->component_lists[component_id].component_size = size;
    ui32 aSize                                         = size + ECS_TAG_SIZE;

    self->component_lists[component_id].components =
      calloc(self->capacity, aSize);

    // printf("\n%d size", aSize);

    // unsigned char *tag = (unsigned char
    // *)(self->component_lists[component_id].components+size); *tag =
    // ECS_TAG_UNUSED;
}

void
ecs_event(ECS *self, enum ECSEvent event)
{

    // Loop through each system, fire the appropriate event
    for (int i = 0; i < self->systems_size; i++) {
        ECSSubscriber func = self->systems[i].subscribers[event];
        if (func == NULL) {
            // func();
            continue;
        }
        // Call the function per-entity
        for (int j = 0; j < self->nextIndex; j++) {
            Entity e = (Entity) {.id = self->ids[j], .index = j, .ecs = self};
            func(e);
        }
    }

    // TODO: determine whether or not there is a required component.
    // Go through that list instead of every entity.
}
