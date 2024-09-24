/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gpak.h"

#include "stdlib.h"

#include "util/filesystem.h"
#include "util/sysdefs.h"

#define _HASHFN _hash_djb2

static u64
_hash_djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

gsk_GPAK
gsk_gpak_init()
{
    gsk_GPAK ret;
    ret.buf_len = 1000; // TODO: for now
    ret.buf     = malloc(sizeof(gsk_GPAK) * ret.buf_len);

    return ret;
}

gsk_GPAK
gsk_gpak_write(gsk_GPAK *p_gpak, const char *str_key_uri, u64 value)
{
    u64 hash         = _HASHFN(str_key_uri);
    u64 idx          = hash % p_gpak->buf_len;
    p_gpak->buf[idx] = value;
}

u64
gsk_gpak_read(gsk_GPAK *p_gpak, const char *str_uri)
{
    u64 hash = _HASHFN(str_uri);
    u64 idx  = hash % p_gpak->buf_len;
    return p_gpak->buf[idx];
}