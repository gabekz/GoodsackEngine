/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "array_list.h"

#include <stdlib.h>
#include <string.h>

#include "util/logger.h"
#include "util/sysdefs.h"

ArrayList
array_list_init(const u32 data_size)
{
    const u32 starting_count = ARRAY_LIST_SIZE;
    size_t buffersize        = data_size * starting_count;

    LOG_INFO("init arraylist");

    ArrayList ret = {
      .list_count       = starting_count,
      .list_next        = 0,
      .is_list_empty    = TRUE,
      .data.buffer      = malloc(buffersize),
      .data.buffer_size = buffersize,
      .data.data_size   = data_size,
    };

    return ret;
}

void
array_list_push(ArrayList *self, void *data)
{
    if (self->list_next >= self->list_count) {
        // LOG_CRITICAL("Exceeding arraylist capacity!");
        //((self->list_next + 1) % ARRAY_LIST_SIZE) + ARRAY_LIST_SIZE;

        size_t newsize =
          self->data.buffer_size + (ARRAY_LIST_SIZE * self->data.buffer_size);

        LOG_DEBUG("Resized from %d to %d", self->data.buffer_size, newsize);

        self->data.buffer_size = newsize;
        self->data.buffer      = realloc(self->data.buffer, newsize);
    }

    memcpy((char *)self->data.buffer + (self->list_next * self->data.data_size),
           data,
           self->data.data_size);
    self->list_next++;
    self->is_list_empty = FALSE;
}

void
array_list_pop(ArrayList *self)
{
    if (self->is_list_empty) {
        LOG_WARN("Trying to pop empty arraylist");
        return; // nothing to pop.
    }

    if (((int)self->list_next - 1) < 0) {
        self->is_list_empty = TRUE;
        self->list_next     = 0;
        return;
    }
    self->list_next--;
}