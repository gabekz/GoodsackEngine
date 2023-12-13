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
array_list_init(const u32 data_size, const u32 list_increment)
{
    if (list_increment <= 0) {
        LOG_CRITICAL("arraylist increment cannot be zero");
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

void
array_list_push(ArrayList *self, void *data)
{
    if (self->list_next >= self->list_count) {

        size_t newsize = self->data.buffer_size +
                         (self->list_increment * self->data.buffer_size);

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