/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "ecs.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/sysdefs.h"

#include "entity/v1/builtin/animator/animator.h"
#include "entity/v1/builtin/audio/audio_listener.h"
#include "entity/v1/builtin/audio/audio_source.h"
#include "entity/v1/builtin/camera/camera.h"
#include "entity/v1/builtin/component_test.h"
#include "entity/v1/builtin/model/model_draw.h"
#include "entity/v1/builtin/transform/transform.h"

// Physics
#include "entity/v1/builtin/physics/collider_setup-system.h"
#include "entity/v1/builtin/physics/rigidbody-system.h"

#if USING_GENERATED_COMPONENTS
#define COMPONENTS_GEN_IMPLEMENTATION
#include "entity/__generated__/components_gen.h"
#endif

gsk_ECS *
gsk_ecs_init(gsk_Renderer *renderer)
{
    gsk_ECS *ecs = malloc(sizeof(gsk_ECS));

    // Initialize entity capacity
    u32 capacity  = 64;
    ecs->ids       = malloc(capacity * sizeof(gsk_EntityId));
    ecs->capacity  = capacity;
    ecs->nextId    = 1;
    ecs->nextIndex = 0;

    ecs->renderer = renderer;

    // Initialize systems and components
    ecs->systems_size = 0;
    ecs->systems      = malloc(1 * sizeof(gsk_ECSSystem));

    s_transform_init(ecs);

    s_camera_init(ecs);
    s_model_draw_init(ecs);
    s_audio_listener_init(ecs);
    s_audio_source_init(ecs);
    s_animator_init(ecs);

    // Physics Systems
    s_collider_setup_system_init(ecs);
    s_rigidbody_system_init(ecs);

#if USING_GENERATED_COMPONENTS
    _ecs_init_internal_gen(ecs);
#else
    _ECS_DECL_COMPONENT(ecs, C_TEST, sizeof(struct ComponentTest));
#endif

    return ecs;
}

gsk_Entity
gsk_ecs_new(gsk_ECS *self)
{
    u32 capacity = self->capacity;
    // check if we have available capcity
    if (capacity < self->nextId) {
        capacity *= 2;
        // TODO: Reallocate component lists
    }

    gsk_Entity entity =
      (gsk_Entity) {.id    = self->nextId,
                .index = self->nextIndex, // TODO: alignment (for deletion)
                .ecs   = self};

    // TODO: Fill next available slot (if deletion)

    self->ids[self->nextId - 1] = entity.id;
    self->nextId++;
    self->nextIndex++;

    return entity;
}

void
_gsk_ecs_add_internal(gsk_Entity entity, u32 component_id, void *value)
{
    gsk_ECS *ecs               = entity.ecs;
    gsk_ECSComponentList *list = &ecs->component_lists[component_id];
    u32 size              = (entity.index * ECS_TAG_SIZE) +
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
    u32 index =
      (entity.index * ECS_TAG_SIZE) + (list->component_size * (entity.index));
    if (value != NULL) {
        memcpy((char *)((char *)((gsk_ECSComponentList *)list->components) + index),
               value,
               list->component_size);
        // list = realloc(list, list.components_size+1 * sizeof());
    }
}

int
gsk_ecs_has(gsk_Entity entity, ECSComponentType component_id)
{
    gsk_ECSComponentList *list = &entity.ecs->component_lists[component_id];

    u32 size = (entity.index * ECS_TAG_SIZE) +
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
gsk_ecs_get(gsk_Entity entity, ECSComponentType component_id)
{
    assert(gsk_ecs_has(entity, component_id));
    gsk_ECSComponentList *list = &entity.ecs->component_lists[component_id];

    u32 size =
      ((entity.index * ECS_TAG_SIZE) + (list->component_size * (entity.index)));
    // size = size - (list->component_size - 1);
    //(list->component_size * (entity.index));
    // printf("\necs_get - id: %d, index %d, id: %d", component_id, size,
    // entity.id);
    return (
      char *)((char *)(gsk_ECSComponentList *)(entity.ecs
                                             ->component_lists[component_id]
                                             .components) +
              size);
    // return ECSCL_GET(&entity.ecs->component_lists[component_id], entity.id);
}

void
gsk_ecs_system_register(gsk_ECS *self, gsk_ECSSystem system)
{
    u32 newSize = self->systems_size + 1;

    gsk_ECSSystem *p  = realloc(self->systems, newSize * sizeof(gsk_ECSSystem));
    self->systems = p;

    self->systems[newSize - 1] = system;
    self->systems_size         = newSize;
}

void
gsk_ecs_component_register(gsk_ECS *self, u32 component_id, u64 size)
{
    self->component_lists[component_id].component_size = size;
    u32 aSize                                         = size + ECS_TAG_SIZE;

    self->component_lists[component_id].components =
      calloc(self->capacity, aSize);

    // printf("\n%d size", aSize);

    // unsigned char *tag = (unsigned char
    // *)(self->component_lists[component_id].components+size); *tag =
    // ECS_TAG_UNUSED;
}

void
gsk_ecs_event(gsk_ECS *self, enum ECSEvent event)
{

    // Loop through each system, fire the appropriate event
    for (int i = 0; i < self->systems_size; i++) {
        gsk_ECSSubscriber func = self->systems[i].subscribers[event];
        if (func == NULL) {
            // func();
            continue;
        }
        // Call the function per-entity
        for (int j = 0; j < self->nextIndex; j++) {
            gsk_Entity e = (gsk_Entity) {.id = self->ids[j], .index = j, .ecs = self};
            func(e);
        }
    }

    // TODO: determine whether or not there is a required component.
    // Go through that list instead of every entity.
}
