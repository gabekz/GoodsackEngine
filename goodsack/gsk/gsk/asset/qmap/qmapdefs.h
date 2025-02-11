/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __QMAPDEFS_H__
#define __QMAPDEFS_H__

#include "core/graphics/mesh/model.h"
#include "core/graphics/texture/texture_set.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*************************************************************************
 * QMap defs
 *************************************************************************/

#define QMAP_MAX_FIELD_MEMBERS 20

// classification
#define QMAP_CLASS_BACK     0
#define QMAP_CLASS_FRONT    1
#define QMAP_CLASS_ON_PLANE 3

#define QMAP_IMPORT_SCALE 0.02f
//#define QMAP_IMPORT_SCALE 1.0f

#define QMAP_DEFAULT_TEXTURE_SIZE 512.0f

#define QMAP_ALLOC_ITER 1 // default number for realloc'ing bloks

#define QMAP_NOEXPORT_FIELD_STR "_tb_layer_omit_from_export"

/*************************************************************************
 * QMap types
 *************************************************************************/

typedef struct gsk_QMapPolygonVertex
{
    vec3 position;
    vec2 texture;
    vec3 normal;
    vec3 tangent;
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
    vec2 tex_dimensions; /* grabbed from gsk_Texture data */
    f32 tex_rotation;
    char tex_name[256];

} gsk_QMapPlane;

typedef struct gsk_QMapBrush
{
    s32 brush_index;
    ArrayList list_planes;
    ArrayList list_polygons;

    vec3 brush_bounds[2];
    vec3 world_pos;

} gsk_QMapBrush;

typedef struct gsk_QMapEntityField
{
    f32 members[QMAP_MAX_FIELD_MEMBERS];
    s32 field_type;
    char key[256];
    char value[256];
} gsk_QMapEntityField;

typedef struct gsk_QMapEntity
{
    s32 ent_index;
    ArrayList list_brushes;
    ArrayList list_fields;
} gsk_QMapEntity;

typedef struct gsk_QMapContainer
{
    ArrayList list_entities;

    gsk_QMapEntity *p_cnt_entity;
    gsk_QMapBrush *p_cnt_brush;

    s32 total_entities;
    s32 total_brushes;
    s32 total_planes;

    u8 is_map_compiled, is_model_loaded;

    gsk_Model *p_model;
    gsk_TextureSet *p_texture_set;

} gsk_QMapContainer;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __QMAPDEFS_H__