/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __IO_ASSET_H__
#define __IO_ASSET_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_IO_Asset
{
    void *buff;
    int buff_len;
    int asset_type;
} gsk_IO_Asset;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __IO_ASSET_H__