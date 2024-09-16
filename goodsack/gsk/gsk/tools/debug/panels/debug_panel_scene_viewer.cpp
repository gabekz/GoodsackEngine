/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_scene_viewer.hpp"
#include <imgui.h>

void
gsk::tools::panels::SceneViewer::draw(void)
{
    using namespace ImGui;

    Text("Total Scene Count: %d", p_renderer->sceneC);
    Separator();
    Text("Active Scene (index): %d", p_renderer->activeScene);

    PushButtonRepeat(true);
    if (ArrowButton("##left", ImGuiDir_Left)) { scene_queued--; }
    SameLine(0.0f, GetStyle().ItemInnerSpacing.x);
    if (ArrowButton("##right", ImGuiDir_Right)) { scene_queued++; }
    PopButtonRepeat();
    SameLine();
    Text("index %d", scene_queued);

    SameLine();
    if (Button("Load Scene"))
    {
        if (scene_queued > p_renderer->sceneC || scene_queued < 0)
        {
            scene_queued = 0;
        } else
        {
            gsk_renderer_active_scene(p_renderer, scene_queued);
            gsk_renderer_start(p_renderer);
            scene_queued = p_renderer->activeScene;
        }
    }
}