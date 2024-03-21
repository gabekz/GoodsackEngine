/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_QMAP_H__
#define __LOADER_QMAP_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_QMapContainer
{
    int a;

} gsk_QMapContainer;

typedef struct gsk_QMapEntity
{
    int a;

} gsk_QMapEntity;

typedef struct gsk_QMapBrush
{
    int a;

} gsk_QMapBrush;

gsk_QMapContainer
gsk_load_qmap(const char *map_path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_QMAP_H__