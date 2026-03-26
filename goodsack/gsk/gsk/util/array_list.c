/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "array_list.h"

#include <stdlib.h>
#include <string.h>

#include "util/logger.h"
#include "util/sysdefs.h"

#define LOG_ENABLE            FALSE
#define FALLBACK_CAPACITY     1
#define RESERVE_ITERATION_CAP 100

static ArrayList
__array_list_new_internal(const u32 data_size, const u32 list_increment)
{
    u32 increment = list_increment;

    if (data_size <= 0)
    {
        LOG_CRITICAL("Failed to create arraylist. Data size is %d", data_size);
    }

    if (list_increment <= 0)
    {
        LOG_WARN("array_list increment cannot be zero (is %d)", list_increment);
        increment = FALLBACK_CAPACITY;
    }

    const u32 starting_count = increment;
    size_t buffersize        = data_size * starting_count;

    ArrayList ret = {
      .list_capacity    = starting_count,
      .list_next        = 0,
      .list_increment   = increment,
      .is_list_empty    = TRUE,
      .data.buffer      = malloc(buffersize),
      .data.buffer_size = buffersize,
      .data.data_size   = data_size,
    };

    return ret;
}

static ArrayList
__array_list_reset_internal(ArrayList *p_self)
{
    u32 data_size       = p_self->data.data_size;
    u32 list_incremenet = p_self->list_increment;

    free(p_self->data.buffer);
    *p_self = __array_list_new_internal(data_size, list_incremenet);
}

static void
__array_list_reserve_internal(ArrayList *self, u32 count)
{
    u8 reserve_iterations = 0;

    if (self == NULL || count == 0) { return; }

    u32 required_capacity = self->list_next + count;
    if (required_capacity <= self->list_capacity) { return; }

    u32 old_capacity    = self->list_capacity;
    u32 old_buffer_size = self->data.buffer_size;

    u32 missing = required_capacity - self->list_capacity;

    u32 increments =
      (missing + self->list_increment - 1) / self->list_increment;

    // TODO: proper reserve cap
    // if(increments >= RESERVE_ITERATION_CAP) {

    self->list_capacity += increments * self->list_increment;
    self->data.buffer_size = self->list_capacity * self->data.data_size;

    void *p = realloc(self->data.buffer, self->data.buffer_size);
    if (p == NULL)
    {
        LOG_CRITICAL("Failed to reallocate array_list %p", (void *)self);
    }
    self->data.buffer = p;
}

ArrayList
array_list_init(const u32 data_size, const u32 list_increment)
{
    return __array_list_new_internal(data_size, list_increment);
}

void
array_list_append(ArrayList *self, void *data, u32 data_count)
{
    if (self == NULL)
    {
        LOG_ERROR("Failed to push arraylist, list (%p) is NULL", self);
        return;
    }

    // ensure we have enough memory reserved
    __array_list_reserve_internal(self, data_count);

    if (data != NULL)
    {
        memcpy((char *)self->data.buffer +
                 (self->list_next * self->data.data_size),
               data,
               self->data.data_size * data_count);
    }

    self->list_next += data_count;
    self->is_list_empty = FALSE;
}

void
array_list_push(ArrayList *self, void *data)
{
    array_list_append(self, data, 1);
}

void
array_list_pop(ArrayList *self)
{
    if (self == NULL)
    {
        LOG_ERROR("Failed to pop arraylist, list (%p) is NULL", self);
        return;
    }

    if (self->is_list_empty)
    {
        LOG_WARN("Trying to pop empty arraylist");
        return; // nothing to pop.
    }

    self->list_next--;
    if (((int)self->list_next) <= 0)
    {
        self->is_list_empty = TRUE;
#if ARRAY_LIST_RESIZE_EMPTY
        __array_list_reset_internal(self);
#endif
    }
}

void *
array_list_get_at_index(ArrayList *self, u64 index)
{
    if (self == NULL)
    {
        LOG_ERROR("Failed to get arraylist item, list (%p) is NULL", self);
        return NULL;
    }

    if (self->list_next <= index)
    {
        LOG_ERROR("Failed to retrieve index %d in list of count %d (%p)",
                  index,
                  LIST_COUNT(self),
                  self);

        return NULL;
    }

    void *data = self->data.buffer;

    u32 size = self->data.data_size * index;
    return (char *)data + size;
}

u32
array_list_count(ArrayList *self)
{
    if (self == NULL)
    {
        LOG_ERROR("checking count for NULL arraylist %p", self);
        return 0;
    }

    return (self->is_list_empty) ? 0 : self->list_next - 1;
}

u32
array_list_free(ArrayList *self)
{
    if (self == NULL)
    {
        LOG_ERROR(
          "error freeing array list - checking count for NULL arraylist %p",
          self);
        return 0;
    }

    if (self->data.buffer == NULL)
    {
        LOG_ERROR("error freeing array list - data buffer is NULL");
        return 0;
    }

    free(self->data.buffer);

    self->is_list_empty = TRUE;
}