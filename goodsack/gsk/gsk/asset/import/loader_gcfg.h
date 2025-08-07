/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_GCFG_H__
#define __LOADER_GCFG_H__

#include "asset/asset_gcfg.h"
#include "util/array_list.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_GCFG
gsk_load_gcfg(const char *path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_GCFG_H__