/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
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
#include "entity/modules/physics/collider_debug_draw-system.h"
#include "entity/modules/physics/collider_setup-system.h"
#include "entity/modules/physics/rigidbody-system.h"
#include "entity/modules/physics/rigidbody_forces-system.h"

// Player
#include "entity/modules/player/player_controller-system.h"

// Misc
#include "entity/modules/misc/health_setup.h"
#include "entity/modules/particles_ecs/particles_ecs-system.h"

#if USING_GENERATED_COMPONENTS
#define COMPONENTS_GEN_IMPLEMENTATION
#include "entity/__generated__/components_gen.h"
#endif

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

    /*==== Resize state data =========================================*/

    self->ids =
      __safe_realloc(self->ids, newsize * sizeof(gsk_EntityId), "ids");

    /*==== Resize *ids_init ==========================================*/

    self->ids_init = __safe_realloc(
      self->ids_init, newsize * sizeof(gsk_EntityFlags), "ids_init");

    for (int i = self->nextIndex; i < newsize; i++)
    {
        // default entity flags
        self->ids_init[i] = GskEcsEntityFlag_None;
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
    self->ids_init[entity.index] = GskEcsEntityFlag_None;
    self->ids[entity.index]      = ECS_ID_DELETED;
}

gsk_ECS *
gsk_ecs_init(gsk_Renderer *renderer)
{
    gsk_ECS *ecs = malloc(sizeof(gsk_ECS));

    // Initialize entity capacity
    u32 capacity  = ECS_ENT_CAPACITY;
    ecs->capacity = capacity;

    // initialize id list
    ecs->ids       = malloc(capacity * sizeof(gsk_EntityId));
    ecs->nextId    = ECS_ID_FIRST;
    ecs->nextIndex = 0;

    // initialize init list (list of entities with initialization flag)
    // TODO: gsk_EntityFlag *p_ent_flags
    // TODO: gsk_EntityId *p_ent_ids
    ecs->ids_init = malloc(capacity * sizeof(gsk_EntityFlags));
    for (int i = 0; i < capacity; i++)
    {
        // default entity flags
        ecs->ids_init[i] = GskEcsEntityFlag_None;
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

    // Misc Systems
    s_health_setup_init(ecs);
    s_particles_ecs_system_init(ecs);

    // s_collider_debug_draw_system_init(ecs);

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
        if (self->ids[i] == ECS_ID_DELETED)
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
    self->ids[entity.index] = entity.id;
    self->ids_init[entity.index] |= GskEcsEntityFlag_Enabled;

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
    gsk_EntityFlags *p_flags = &(entity.ecs->ids_init[entity.index]);

    if (is_active == TRUE)
    {
        *p_flags |= GskEcsEntityFlag_Enabled;
        return;
    }

    *p_flags &= ~GskEcsEntityFlag_Enabled;
}

void
gsk_ecs_ent_destroy(gsk_Entity entity)
{
    entity.ecs->ids_init[entity.index] |= GskEcsEntityFlag_Delete;
}

void
_gsk_ecs_add_internal(gsk_Entity entity, u32 component_id, void *value)
{
    gsk_ECS *ecs = entity.ecs;

    if (ecs->ids_init[entity.index] & GskEcsEntityFlag_Initialized)
    {
        LOG_WARN("Cannot add components (yet) to an already initialized entity "
                 "(id: %d).",
                 entity.id);
        return;
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
    // loop through each entity
    for (int i = 0; i < self->nextIndex; i++)
    {
        gsk_Entity ent =
          (gsk_Entity) {.id = self->ids[i], .index = i, .ecs = self};

        gsk_EntityFlags *p_flags = &self->ids_init[i];

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
            gsk_ECSSubscriber func = self->systems[j].subscribers[event];
            if (func != NULL) { func(ent); }
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
}
