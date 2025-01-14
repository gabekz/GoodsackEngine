/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "array_list.h"

#include <stdlib.h>
#include <string.h>

#include "util/logger.h"
#include "util/sysdefs.h"

#define LOG_ENABLE FALSE

static ArrayList
__array_list_new_internal(const u32 data_size, const u32 list_increment)
{
    if (list_increment <= 0)
    {
        LOG_CRITICAL("array_list increment cannot be zero (is %d)",
                     list_increment);
    }

    const u32 starting_count = list_increment;
    size_t buffersize        = data_size * starting_count;

    ArrayList ret = {
      .list_count       = starting_count,
      .list_next        = 0,
      .list_increment   = list_increment,
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

ArrayList
array_list_init(const u32 data_size, const u32 list_increment)
{
    return __array_list_new_internal(data_size, list_increment);
}

void
array_list_push(ArrayList *self, void *data)
{
    if (self->list_next >= self->list_count)
    {

        size_t newsize = self->data.buffer_size +
                         (self->list_increment * self->data.data_size);

        size_t newcount = self->list_count + self->list_increment;

#if LOG_ENABLE
        LOG_TRACE("Resized list from %d to %d. Buffer went from %d to %d",
                  self->list_count,
                  newcount,
                  self->data.buffer_size,
                  newsize);
#endif

        void *p = realloc(self->data.buffer, newsize);
        if (p == NULL)
        {
            LOG_CRITICAL("Failed to reallocate array_list %p", (void *)self);
        }
        self->data.buffer      = p;
        self->data.buffer_size = newsize;
        self->list_count       = newcount;
    }

    if (data != NULL)
    {
        memcpy((char *)self->data.buffer +
                 (self->list_next * self->data.data_size),
               data,
               self->data.data_size);
    }

    self->list_next++;
    self->is_list_empty = FALSE;
}

void
array_list_pop(ArrayList *self)
{
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
    if (self->list_next <= index)
    {
        LOG_CRITICAL(
          "Failed to retrieve item in array list. Unchecked data not allowed!");
    }

    void *data = self->data.buffer;

    u32 size = self->data.data_size * index;
    return (char *)data + size;
}