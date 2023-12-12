/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ARRAY_LIST_H__
#define __ARRAY_LIST_H__

#include "util/sysdefs.h"

#define ARRAY_LIST_SIZE 8

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct ArrayList
{
    u64 list_count, list_next;
    byte_t is_list_empty;
    struct
    {
        void *buffer;
        size_t buffer_size, data_size;
    } data;
} ArrayList;

ArrayList
array_list_init(const u32 data_size);

void
array_list_push(ArrayList *self, void *data);

void
array_list_pop(ArrayList *self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ARRAY_LIST_H__