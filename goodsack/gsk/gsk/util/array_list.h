/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ARRAY_LIST_H__
#define __ARRAY_LIST_H__

#include <stdlib.h>

#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define ARRAY_LIST_RESIZE_EMPTY FALSE

#define LIST_INIT(a, b) array_list_init(a, b)
#define LIST_PUSH(a, b) array_list_push(a, b)
#define LIST_GET(a, b)  array_list_get_at_index(a, b)
#define LIST_COUNT(a)   array_list_count(a)

typedef struct ArrayList
{
    u64 list_capacity;
    u64 list_next;
    u32 list_increment; // realloc growth
    byte_t is_list_empty;
    struct
    {
        void *buffer;
        size_t buffer_size, data_size;
    } data;
} ArrayList;

ArrayList
array_list_init(const u32 data_size, const u32 list_increment);

void
array_list_push(ArrayList *self, void *data);

void
array_list_pop(ArrayList *self);

void *
array_list_get_at_index(ArrayList *self, u64 index);

u32
array_list_count(ArrayList *self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ARRAY_LIST_H__