/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "ecs.h"

#define _PROFILE 0

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/sysdefs.h"
#if _PROFILE
#include "util/timer.h"
#endif //_PROFILE

#if USING_GENERATED_COMPONENTS
#define COMPONENTS_GEN_IMPLEMENTATION
//#include "entity/__generated__/components_gen.h"
#include "gsk_generated/ecs_components_gen.h"
#endif // USING_GENERATED_COMPONENTS

// subroutine to safely reallocate data to new_size
static void *
__safe_realloc(void *ptr, size_t new_size, const char *error_type_name)
{
    void *p = realloc(ptr, new_size);
    if (p == NULL)
    {
        LOG_CRITICAL("Failed to reallocate %s (%p)", error_type_name, p);
    }
    return p;
}

/**
 * @brief           Reallocates ECS data if capacity is reached.
 *
 * @param[in] self  Pointer to gsk_ECS
 * @return          TRUE if reallocated
 * @return          FALSE if capacity was not reached.
 */
static u8
__check_reallocate_ecs(gsk_ECS *self)
{
    if (self->nextIndex < self->capacity) { return FALSE; }

    u32 newsize = self->capacity * 2;

    LOG_DEBUG("Resizing ECS Data cap from %d to %d", self->capacity, newsize);

    /*==== Resize *p_ent_ids =========================================*/

    self->p_ent_ids = __safe_realloc(
      self->p_ent_ids, newsize * sizeof(gsk_EntityId), "p_ent_ids");

    /*==== Resize *p_ent_flags =======================================*/

    self->p_ent_flags = __safe_realloc(
      self->p_ent_flags, newsize * sizeof(gsk_EntityFlags), "p_ent_flags");

    for (int i = self->nextIndex; i < newsize; i++)
    {
        // default entity flags
        self->p_ent_flags[i] = GskEcsEntityFlag_None;
    }

    /*==== Resize *p_ent_layers ======================================*/

    self->p_ent_layers = __safe_realloc(
      self->p_ent_layers, newsize * sizeof(gsk_EntityLayer), "p_ent_layers");

    for (int i = self->nextIndex; i < newsize; i++)
    {
        // default entity flags
        self->p_ent_layers[i] = 0;
    }

    /*==== Resize *entity_names ======================================*/

    self->entity_names = __safe_realloc(
      self->entity_names, newsize * sizeof(char *), "entity_names");

    /*---- create new strings for each new name ------------------*/
    for (int i = self->nextIndex; i < newsize; i++)
    {
        self->entity_names[i] = malloc(sizeof(char) * ECS_NAME_LEN_MAX);
        snprintf(self->entity_names[i], 16, "Entity_%d", i);
    }

    /*==== Resize *component_lists ===================================*/

    for (int i = 0; i < ECSCOMPONENT_LAST + 1; i++)
    {
        u32 cmpsize = self->component_lists[i].component_size + ECS_TAG_SIZE;

        self->component_lists[i].components =
          __safe_realloc(self->component_lists[i].components,
                         newsize * cmpsize,
                         "component_list");

        /*---- set each component as ECS_TAG_UNUSED --------------*/

        for (int j = self->nextIndex; j < newsize; j++)
        {
            u32 size = (j * ECS_TAG_SIZE) +
                       (self->component_lists[i].component_size * (j + 1));

            char *tag =
              (char *)((void *)self->component_lists[i].components) + size;
            *tag = ECS_TAG_UNUSED;
        }
    }

    self->capacity = newsize;
    return TRUE;
}

// mark entity as NONE + disables associated components
static void
__ent_mark_deleted(gsk_ECS *self, gsk_Entity entity)
{
    for (int i = 0; i < ECSCOMPONENT_LAST + 1; i++)
    {
        _gsk_ecs_set_internal(entity, i, FALSE);
    }
    self->p_ent_ids[entity.index]    = ECS_ID_DELETED;
    self->p_ent_flags[entity.index]  = GskEcsEntityFlag_None;
    self->p_ent_layers[entity.index] = 0;
}

gsk_ECS *
gsk_ecs_init(gsk_Renderer *renderer)
{
    gsk_ECS *ecs = malloc(sizeof(gsk_ECS));

    // Initialize entity capacity
    u32 capacity  = ECS_ENT_CAPACITY;
    ecs->capacity = capacity;

    // initialize id list
    ecs->p_ent_ids = malloc(capacity * sizeof(gsk_EntityId));
    ecs->nextId    = ECS_ID_FIRST;
    ecs->nextIndex = 0;

    // initialize init list (list of entities with initialization flag)
    ecs->p_ent_flags  = malloc(capacity * sizeof(gsk_EntityFlags));
    ecs->p_ent_layers = malloc(capacity * sizeof(gsk_EntityLayer));
    for (int i = 0; i < capacity; i++)
    {
        // default entity flags
        ecs->p_ent_flags[i] = GskEcsEntityFlag_None;

        // default entity layer
        ecs->p_ent_layers[i] = 0;
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

    // set current event
    ecs->current_event = ECS_INIT;

    // Initialize systems and components
    ecs->systems_size    = 0;
    ecs->systems         = malloc(sizeof(gsk_ECSSystem));
    ecs->systems_profile = malloc(sizeof(gsk_ECSSystemProfile));

#if USING_GENERATED_COMPONENTS
    _ecs_init_internal_gen(ecs);
#endif

    return ecs;
}

gsk_Entity
_gsk_ecs_new_internal(gsk_ECS *self, char *name)
{
    u64 next_index  = self->nextIndex; // nextIndex by default
    u8 is_replacing = FALSE;

    // Find possible replacement
    // TODO: optimize by not going through each entity, just a "delete-list"
    for (int i = 0; i < self->nextIndex; i++)
    {
        if (self->p_ent_ids[i] == ECS_ID_DELETED)
        {
            next_index   = (u64)i;
            is_replacing = TRUE;
            break;
        }
    }

    if (is_replacing == FALSE) { __check_reallocate_ecs(self); }

    gsk_Entity entity =
      (gsk_Entity) {.id = self->nextId, .index = next_index, .ecs = self};

    // Enable the entity
    self->p_ent_ids[entity.index] = entity.id;
    self->p_ent_flags[entity.index] |= GskEcsEntityFlag_Enabled;

    // only iterate nextIndex if we did not replace an old entity
    if (is_replacing == FALSE) { self->nextIndex++; }

    // Assign name if passed in
    if (name != NULL) { strcpy(self->entity_names[entity.index], name); }

    // No matter what, increment nextId
    self->nextId++;

    return entity;
}

void
gsk_ecs_ent_set_active(gsk_Entity entity, u8 is_active)
{
    gsk_EntityFlags *p_flags = &(entity.ecs->p_ent_flags[entity.index]);

    if (is_active == TRUE)
    {
        *p_flags |= GskEcsEntityFlag_Enabled;
        return;
    }

    *p_flags &= ~GskEcsEntityFlag_Enabled;
}

void
gsk_ecs_ent_set_layer(gsk_Entity entity, gsk_EntityLayer layer)
{
    if (layer > ECS_MAX_LAYERS)
    {
        LOG_ERROR("exceeded maximum layer count (%d). Setting entity (id: %d) "
                  "layer to 0",
                  ECS_MAX_LAYERS,
                  entity.id);
        return;
    }

    entity.ecs->p_ent_layers[entity.index] = layer;
}

void
gsk_ecs_ent_destroy(gsk_Entity entity)
{
    entity.ecs->p_ent_flags[entity.index] |= GskEcsEntityFlag_Delete;
}

void
_gsk_ecs_add_internal(gsk_Entity entity, u32 component_id, void *value)
{
    gsk_ECS *ecs = entity.ecs;

    if (ecs->p_ent_flags[entity.index] & GskEcsEntityFlag_Initialized)
    {
        // TODO: Leaving this warning here, because we want to have a
        // "component-flag" that checks for a components requirement for
        // initialization (on a per-component scale)
        //
        // gsk_ecs_get() should check if initialization to the component has
        // been done already by a system, and we may want to set up macros that
        // are specific to the ECS_EVENT
        LOG_WARN("adding component to an already initialized entity "
                 "(id: %d).",
                 entity.id);
    }

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

#if 1
    // ensure that the component only exists AFTER the entity is initialized
    gsk_EntityFlags *p_flags = &entity.ecs->p_ent_flags[entity.index];
    u8 is_ent_initialized    = (*p_flags & GskEcsEntityFlag_Initialized);
    if (entity.ecs->current_event != ECS_INIT && is_ent_initialized == FALSE)
    {
        return FALSE;
    }
#endif

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

    gsk_ECSSystem *q =
      realloc(self->systems_profile, newsize * sizeof(gsk_ECSSystemProfile));
    self->systems_profile = q;

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
}

gsk_Entity
gsk_ecs_ent(gsk_ECS *self, gsk_EntityId id)
{
    for (int i = 0; i < self->nextIndex; i++)
    {
        if (self->p_ent_ids[i] == id)
        {
            // LOG_INFO("got\t id: %d\t index: %d", self->ids[i], i);

            return (gsk_Entity) {
              .id    = self->p_ent_ids[i],
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
gsk_ecs_event(gsk_ECS *self, s32 event)
{
    // set current event
    self->current_event = event;

    // reset system times
    for (int j = 0; j < self->systems_size; j++)
    {
        self->systems_profile[j].subscribers[event] = 0.0f;
    }

    // loop through each entity
    for (int i = 0; i < self->nextIndex; i++)
    {
        gsk_Entity ent =
          (gsk_Entity) {.id = self->p_ent_ids[i], .index = i, .ecs = self};

        gsk_EntityFlags *p_flags = &self->p_ent_flags[i];

        u8 is_ent_enabled     = (*p_flags & GskEcsEntityFlag_Enabled);
        u8 is_ent_delete      = (*p_flags & GskEcsEntityFlag_Delete);
        u8 is_ent_initialized = (*p_flags & GskEcsEntityFlag_Initialized);

        if (event != ECS_INIT && !is_ent_initialized) { continue; }
        if (event != ECS_DESTROY && !is_ent_enabled) { continue; }

        if (event == ECS_DESTROY && is_ent_delete && !is_ent_initialized)
        {
            __ent_mark_deleted(self, ent);
            continue;
        }

        if (event == ECS_DESTROY && !is_ent_delete) { continue; }
        if (event == ECS_INIT && is_ent_initialized) { continue; }

        // Loop through each system, fire the appropriate event
        for (int j = 0; j < self->systems_size; j++)
        {
#if _PROFILE
            f64 start = 0, end = 0;
            start = gsk_timer_get();
#endif //_PROFILE

            gsk_ECSSubscriber func = self->systems[j].subscribers[event];

            self->systems_profile[j].subscribers[event];

            if (func != NULL) { func(ent); }

#if _PROFILE
            end = gsk_timer_get();
            self->systems_profile[j].subscribers[event] +=
              (end - start) * 1000.0;
#endif //_PROFILE
        }

        // ensure we flagged the entity as deleted
        if (event == ECS_DESTROY && is_ent_delete)
        {
            __ent_mark_deleted(self, ent);
            continue;
        }

        // set the initialization flag
        *p_flags |= GskEcsEntityFlag_Initialized;
    }

    // TODO: determine whether or not there is a required component.
    // Go through that list instead of every entity.

#if _PROFILE
    for (int i = 0; i < self->systems_size; i++)
    {

        f64 time = self->systems_profile[i].subscribers[event];

        if (time < 1.0) { continue; }

        LOG_INFO("%d - %f", i, self->systems_profile[i].subscribers[event]);
    }
#endif //_PROFILE
}

const char *
gsk_ecs_get_component_name(ECSComponentType component_id)
{
    return _ECSCOMPONENT_NAMES[component_id];
}