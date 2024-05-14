/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOADER_QMAP_H__
#define __LOADER_QMAP_H__

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"


#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//#define QMAP_TYPE 0

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

    u8 is_map_compiled, is_model_loaded;

    gsk_Model *p_model;
    gsk_TextureSet *p_texture_set;

} gsk_QMapContainer;

gsk_QMapContainer
gsk_qmap_load(const char *map_path, gsk_TextureSet *p_texture_set);

gsk_Model *
gsk_qmap_load_model(gsk_QMapContainer *p_container,
                    gsk_ShaderProgram *p_shader);

#if 0
void *
gsk_qmap_attach_textures(gsk_QMapContainer *p_container, void *p_texture_set);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LOADER_QMAP_H__