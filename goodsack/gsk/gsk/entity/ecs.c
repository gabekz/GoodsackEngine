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

#include "entity/modules/animator/animator.h"
#include "entity/modules/audio/audio_listener.h"
#include "entity/modules/audio/audio_source.h"
#include "entity/modules/camera/camera.h"
#include "entity/modules/model/model_draw.h"
#include "entity/modules/transform/transform.h"

// Physics
#include "entity/modules/physics/collider_setup-system.h"
#include "entity/modules/physics/rigidbody-system.h"
#include "entity/modules/physics/rigidbody_forces-system.h"

// Player
#include "entity/modules/player/player_controller-system.h"

#if USING_GENERATED_COMPONENTS
#define COMPONENTS_GEN_IMPLEMENTATION
#include "entity/__generated__/components_gen.h"
#endif

gsk_ECS *
gsk_ecs_init(gsk_Renderer *renderer)
{
    gsk_ECS *ecs = malloc(sizeof(gsk_ECS));

    // Initialize entity capacity
    u32 capacity  = ECS_ENT_CAPACITY;
    ecs->capacity = capacity;

    // initialize id list
    ecs->ids       = malloc(capacity * sizeof(gsk_EntityId));
    ecs->nextId    = ECS_FIRST_ID;
    ecs->nextIndex = 0;

    // initialize init list (list of entities with initialization flag)
    // TODO: gsk_EntityFlag *p_ent_flags
    // TODO: gsk_EntityId *p_ent_ids
    ecs->ids_init = malloc(capacity * sizeof(gsk_EntityId));
    for (int i = 0; i < capacity; i++)
    {
        // default entity flags
        ecs->ids_init[i] = 0;
    }

    // Create Entity names cache
    const s32 def_name_size = 16;
    ecs->entity_names       = malloc(sizeof(char *) * capacity);

    for (int i = 0; i < capacity; i++)
    {
        ecs->entity_names[i] = malloc(sizeof(char) * ECS_NAME_LEN_MAX);
        snprintf(ecs->entity_names[i], def_name_size, "Entity_%d", i);
    }

    // point to renderer
    ecs->renderer = renderer;

    // Initialize systems and components
    ecs->systems_size = 0;
    ecs->systems      = malloc(sizeof(gsk_ECSSystem));

    s_transform_init(ecs);

    s_camera_init(ecs);
    s_model_draw_init(ecs);
    s_audio_listener_init(ecs);
    s_audio_source_init(ecs);
    s_animator_init(ecs);

    // Physics Systems
    // order is important here..
    s_rigidbody_forces_system_init(ecs); // apply external forces
    s_collider_setup_system_init(ecs);   // check for collisions
    s_rigidbody_system_init(ecs);        // run solvers on collisions. integrate

    // Player Controller
    s_player_controller_system_init(ecs);

#if USING_GENERATED_COMPONENTS
    _ecs_init_internal_gen(ecs);
#endif

    return ecs;
}

gsk_Entity
_gsk_ecs_new_internal(gsk_ECS *self, char *name)
{
    u32 capacity = self->capacity;
    // check if we have available capcity
    if (capacity < self->nextIndex)
    {
        capacity *= 2;
        // TODO: Reallocate component lists
        // TODO: Realloc names list
        // TODO?: Realloc init_list
    }

    gsk_Entity entity =
      (gsk_Entity) {.id    = self->nextId,
                    .index = self->nextIndex, // TODO: alignment (for deletion)
                    .ecs   = self};

    // set initialization flag
    self->ids_init[entity.index] = 0;

    // TODO: Fill next available slot (if deletion)

    self->ids[self->nextIndex] = entity.id;
    self->nextId++;
    self->nextIndex++;

    // Assign name if passed in
    if (name != NULL) { strcpy(self->entity_names[entity.index], name); }

    return entity;
}

void
_gsk_ecs_add_internal(gsk_Entity entity, u32 component_id, void *value)
{
    gsk_ECS *ecs               = entity.ecs;
    gsk_ECSComponentList *list = &ecs->component_lists[component_id];
    u32 size                   = (entity.index * ECS_TAG_SIZE) +
               (list->component_size * (entity.index + 1));

    char *tag = (char *)((void *)list->components) + size;
    *tag |= ECS_TAG_USED;

    u32 index =
      (entity.index * ECS_TAG_SIZE) + (list->component_size * (entity.index));
    if (value != NULL)
    {
        memcpy(
          (char *)((char *)((gsk_ECSComponentList *)list->components) + index),
          value,
          list->component_size);
        // list = realloc(list, list.components_size+1 * sizeof());
    }
}

s32
_gsk_ecs_set_internal(gsk_Entity entity, u32 component_id, u8 is_active)
{
    gsk_ECS *ecs               = entity.ecs;
    gsk_ECSComponentList *list = &ecs->component_lists[component_id];
    u32 size                   = (entity.index * ECS_TAG_SIZE) +
               (list->component_size * (entity.index + 1));

    char *tag = (char *)((void *)list->components) + size;
    *tag      = (is_active) ? ECS_TAG_USED : ECS_TAG_UNUSED;
    s32 value = *tag;

    return value;
}

int
gsk_ecs_has(gsk_Entity entity, ECSComponentType component_id)
{
    if (entity.ecs == NULL) { return 0; }

    gsk_ECSComponentList *list = &entity.ecs->component_lists[component_id];

    u32 size = (entity.index * ECS_TAG_SIZE) +
               (list->component_size * (entity.index + 1));

    char *tag = (char *)((void *)list->components) + size;
    int value = *tag;

    return (*tag == ECS_TAG_USED) ? 1 : 0;
}

void *
gsk_ecs_get(gsk_Entity entity, ECSComponentType component_id)
{
    assert(gsk_ecs_has(entity, component_id));
    gsk_ECSComponentList *list = &entity.ecs->component_lists[component_id];

    u32 size =
      ((entity.index * ECS_TAG_SIZE) + (list->component_size * (entity.index)));

    return (
      char *)((char *)(gsk_ECSComponentList *)(entity.ecs
                                                 ->component_lists[component_id]
                                                 .components) +
              size);
}

void
gsk_ecs_system_register(gsk_ECS *self, gsk_ECSSystem system)
{
    u32 newsize = self->systems_size + 1;

    gsk_ECSSystem *p = realloc(self->systems, newsize * sizeof(gsk_ECSSystem));
    self->systems    = p;

    self->systems[newsize - 1] = system;
    self->systems_size         = newsize;
}

void
gsk_ecs_component_register(gsk_ECS *self, u32 component_id, u64 size)
{
    self->component_lists[component_id].component_size = size;
    u32 aSize                                          = size + ECS_TAG_SIZE;

    self->component_lists[component_id].components =
      calloc(self->capacity, aSize);

    // printf("\n%d size", aSize);

    // unsigned char *tag = (unsigned char
    // *)(self->component_lists[component_id].components+size); *tag =
    // ECS_TAG_UNUSED;
}

gsk_Entity
gsk_ecs_ent(gsk_ECS *self, gsk_EntityId id)
{
    for (int i = 0; i < self->nextIndex; i++)
    {
        if (self->ids[i] == id)
        {
            // LOG_INFO("got\t id: %d\t index: %d", self->ids[i], i);

            return (gsk_Entity) {
              .id    = self->ids[i],
              .index = i,
              .ecs   = self,
            };
        }
    }

    LOG_ERROR("Not found by id %d.", id);
    return (gsk_Entity) {
      .id    = 0,
      .index = 0,
      .ecs   = NULL,
    };
}

void
gsk_ecs_event(gsk_ECS *self, enum ECSEvent event)
{

#if 0
    // Loop through each system, fire the appropriate event
    for (int i = 0; i < self->systems_size; i++)
    {
        gsk_ECSSubscriber func = self->systems[i].subscribers[event];

        if (func == NULL) { continue; }

        // Call the function per-entity
        for (int j = 0; j < self->nextIndex; j++)
        {
            if (event == ECS_INIT)
            {
                if (self->ids_init[j] == 1)
                {
                    LOG_INFO("Entity already initialized");
                    return;
                }
            }

            gsk_Entity e =
              (gsk_Entity) {.id = self->ids[j], .index = j, .ecs = self};
            func(e);
        }
    }
#else

    // loop through each entity
    // Call the function per-entity
    for (int j = 0; j < self->nextIndex; j++)
    {
        // Loop through each system, fire the appropriate event
        for (int i = 0; i < self->systems_size; i++)
        {
            gsk_ECSSubscriber func = self->systems[i].subscribers[event];

            if (func == NULL) { continue; }

            if (event == ECS_INIT)
            {
                if (self->ids_init[j] == 1) { return; }
            }

            gsk_Entity e =
              (gsk_Entity) {.id = self->ids[j], .index = j, .ecs = self};
            func(e);
        }
        self->ids_init[j] = 1;
    }

#endif

    // TODO: determine whether or not there is a required component.
    // Go through that list instead of every entity.
}
