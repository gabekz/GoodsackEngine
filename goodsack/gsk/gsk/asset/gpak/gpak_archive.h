/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_GPAK_ARCHIVE_H__
#define __GSK_GPAK_ARCHIVE_H__

#include "util/array_list.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define GPAK_ARCHIVE(a, b) gsk_archive_bytes(a, b, sizeof(*b))
// #define GPAK_ARCHIVE_ALLOC(a, b, c) gsk_archive_alloc(a, b, c)

typedef enum GskArchiveMode_ {
    GskArchiveMode_Read = 0,
    GskArchiveMode_Write,
} GskArchiveMode_;

typedef u8 GskArchiveMode;

typedef struct gsk_Archive
{
    GskArchiveMode mode;
    u8 is_success;

    void *p_buffer;
    u32 seek_cnt;

    ArrayList out;

} gsk_Archive;

u8
gsk_archive_bytes(gsk_Archive *p_self, void *p_data, u32 data_size);

#if 0
void *
gsk_archive_alloc(gsk_Archive *p_self, void *p_data, u32 data_size);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_GPAK_ARCHIVE_H__