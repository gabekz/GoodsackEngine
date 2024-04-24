/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_QMAP_H__
#define __LOADER_QMAP_H__

#include "core/graphics/mesh/mesh.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_QMapPolygon
{
    ArrayList list_vertices;
} gsk_QMapPolygon;

typedef struct gsk_QMapPlane
{
    vec3 points[3];
    vec3 normal;
    f32 determinant;

    vec2 tex_offset;
    vec2 tex_scale;
    f32 tex_rotation;

} gsk_QMapPlane;

typedef struct gsk_QMapBrush
{
    s32 brush_index;
    ArrayList list_planes;
    ArrayList list_polygons;

} gsk_QMapBrush;

typedef struct gsk_QMapEntity
{
    s32 ent_index;
    ArrayList list_brushes;
} gsk_QMapEntity;

typedef struct gsk_QMapContainer
{
    ArrayList list_entities;

    gsk_QMapEntity *p_cnt_entity;
    gsk_QMapBrush *p_cnt_brush;

    s32 total_entities;
    s32 total_brushes;
    s32 total_planes;

    ArrayList vertices;

    gsk_MeshData *mesh_data;

} gsk_QMapContainer;

gsk_QMapContainer
gsk_load_qmap(const char *map_path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_QMAP_H__