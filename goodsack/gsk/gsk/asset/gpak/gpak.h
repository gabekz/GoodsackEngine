/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GPAK_H__
#define __GPAK_H__

#include "util/filesystem.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_GPakAssetRef
{
    char uri[GSK_FS_MAX_PATH];
    u64 handle;   // default to 0 when not in use
    u8 type;      // asset type
    void *p_next; // chain pointer to next asset in linked-list
} gsk_GPakAssetRef;

typedef struct gsk_GPakContainer
{
    u64 *buf;
    u64 buf_len;
} gsk_GPAK;

gsk_GPAK
gsk_gpak_init();

gsk_GPAK
gsk_gpak_write(gsk_GPAK *p_gpak, const char *str_key_uri, u64 value);

u64
gsk_gpak_read(gsk_GPAK *p_gpak, const char *str_uri);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GPAK_H__