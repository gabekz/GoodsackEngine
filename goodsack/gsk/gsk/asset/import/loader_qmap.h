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

typedef struct gsk_QMapPolygonVertex
{
    vec3 position;
    vec2 texture;
    vec3 normal;
} gsk_QMapPolygonVertex;

typedef struct gsk_QMapPolygon
{
    ArrayList list_vertices;
    vec3 center;

    void *p_mesh_data;
    void *p_texture;
} gsk_QMapPolygon;

typedef struct gsk_QMapPlane
{
    vec3 points[3];
    vec3 uv_axes[2];
    vec3 normal;
    f32 determinant;

    vec2 tex_offset;
    vec2 tex_scale;
    f32 tex_rotation;
    char tex_name[256];

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

    gsk_MeshData *mesh_data;

    void *p_texture_set;

} gsk_QMapContainer;

gsk_QMapContainer
gsk_qmap_load(const char *map_path);

void *
gsk_qmap_attach_texture(gsk_QMapContainer *p_container,
                        void *p_texture,
                        const char *ref_name);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_QMAP_H__