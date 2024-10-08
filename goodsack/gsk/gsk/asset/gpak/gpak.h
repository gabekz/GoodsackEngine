/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GPAK_H__
#define __GPAK_H__

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/sysdefs.h"

#include "asset/asset_cache.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_GpakHeader
{
    char signature[4];
    char version[4];
    u16 file_size;
} gsk_GpakHeader;

typedef struct gsk_GpakHandler
{
    void *p_file;
    u8 is_ready;
    u8 is_compiled;
    u8 mode;

} gsk_GpakHandler;
typedef struct gsk_GpakWriter
{
    void *file_ptr;
    u8 is_ready;

} gsk_GpakWriter;

gsk_GpakHandler
gsk_gpak_handler_init();

gsk_GpakWriter
gsk_gpak_writer_init();

void
gsk_gpak_writer_populate_cache(gsk_GpakWriter *p_writer,
                               gsk_AssetCache *p_cache);
void
gsk_gpak_writer_write(gsk_GpakWriter *p_writer);

void
gsk_gpak_writer_close(gsk_GpakWriter *p_writer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GPAK_H__