/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "scene.h"

void
gsk_scene_add_canvas(gsk_Scene *p_scene,
                     gsk_GuiCanvas canvas,
                     const char *canvas_name)
{
    if (p_scene->total_canvases >= SCENE_MAX_CANVASES)
    {
        LOG_ERROR(
          "Failed to add canvas to scene: exceeded scene canvas limit (%d)",
          SCENE_MAX_CANVASES);

        return;
    }

    if (p_scene->total_canvases <= 0)
    {
        p_scene->canvases_table = hash_table_init(SCENE_MAX_CANVASES);
    }

    hash_table_add(
      &p_scene->canvases_table, canvas_name, (u64)p_scene->total_canvases);

    p_scene->canvases[p_scene->total_canvases] = canvas;
    p_scene->total_canvases++;
}

gsk_GuiCanvas *
gsk_scene_get_canvas_by_name(gsk_Scene *p_scene, const char *canvas_name)
{
    // TODO: validate
    u64 canvas_index = hash_table_get(&p_scene->canvases_table, canvas_name);
    return &p_scene->canvases[canvas_index];
}