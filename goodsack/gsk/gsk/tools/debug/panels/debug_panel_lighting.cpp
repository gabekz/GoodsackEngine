/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_lighting.hpp"

#include <imgui.h>

void
gsk::tools::panels::Lighting::draw(void)
{
    using namespace ImGui;

    gsk_Scene *p_active_scene = p_renderer->sceneL[p_renderer->activeScene];

    Separator();
    Text("Directional Light");
    DragFloat3("Position",
               p_active_scene->lighting_data.lights[0].position,
               0.1f,
               -3000,
               3000);
    ColorEdit3("Light Color", p_active_scene->lighting_data.lights[0].color);
    DragFloat("Light Strength",
              &p_active_scene->lighting_data.lights[0].strength,
              1,
              0,
              100);

    Separator();
    Text("Ambient Light");
    ColorEdit3("Ambient Color",
               p_renderer->lightOptions.ambient_color_multiplier);
    SliderFloat(
      "Ambient Strength", &p_renderer->lightOptions.ambient_strength, 0, 1);
    SliderFloat("Prefilter Influence",
                &p_renderer->lightOptions.prefilter_strength,
                0,
                1);

    Separator();
    Text("Fog");
    DragFloat(
      "Fog Start", &p_active_scene->fogOptions.fog_start, 0.1f, -1, 100);
    DragFloat("Fog End", &p_active_scene->fogOptions.fog_end, 0.1f, 1, 4096);
    DragFloat(
      "Fog Density", &p_active_scene->fogOptions.fog_density, 0.1f, 0.1f, 100);
    ColorEdit3("Fog Color", p_active_scene->fogOptions.fog_color);
}