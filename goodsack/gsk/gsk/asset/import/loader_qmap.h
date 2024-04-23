/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_QMAP_H__
#define __LOADER_QMAP_H__

#include "util/array_list.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_QMapPlane
{
    int a;

} gsk_QMapPlane;

typedef struct gsk_QMapBrush
{
    gsk_QMapPlane planes[20];
    s32 plane_count;

} gsk_QMapBrush;

typedef struct gsk_QMapEntity
{
    ArrayList list_brushes;
    int ent_index;
} gsk_QMapEntity;

typedef struct gsk_QMapContainer
{
    ArrayList list_entities;

    gsk_QMapEntity *p_cnt_entity;
    gsk_QMapBrush *p_cnt_brush;

    s32 total_entities;
    s32 total_brushes;
    s32 total_planes;

} gsk_QMapContainer;

gsk_QMapContainer
gsk_load_qmap(const char *map_path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_QMAP_H__