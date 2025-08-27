/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_SCENE_H__
#define __GSK_SCENE_H__

/* scene steps::
 * 1) Initialize and set active camera
 * 2) Create Light information
 [Standard]
 * 3) Renderer tick [all logic updates + shader updates]
 * 4) Render [all meshes in meshList]
 [ECS]
 * 3) Process Systems
 */

#include "core/graphics/lighting/lighting.h"
#include "core/graphics/lighting/skybox.h"
#include "core/graphics/ui/gui_canvas.h"

#include "util/hash_table.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define SCENE_MAX_CANVASES 16

typedef struct gsk_Scene
{
    u8 has_skybox;
    u32 id, meshC, lightC;

    struct gsk_ECS *ecs;
    gsk_Skybox *skybox;

    gsk_LightingData lighting_data;

    HashTable canvases_table;
    gsk_GuiCanvas canvases[SCENE_MAX_CANVASES];
    u32 total_canvases;

    struct
    {
        f32 fog_start, fog_end, fog_density;
        vec3 fog_color;
    } fogOptions;

} gsk_Scene;

void
gsk_scene_add_canvas(gsk_Scene *p_scene,
                     gsk_GuiCanvas canvas,
                     const char *canvas_name);

gsk_GuiCanvas *
gsk_scene_get_canvas_by_name(gsk_Scene *p_scene, const char *canvas_name);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_SCENE_H__
