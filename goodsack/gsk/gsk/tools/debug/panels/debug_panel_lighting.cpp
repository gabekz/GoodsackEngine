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

    Separator();
    Text("Directional Light");
    DragFloat3("Position",
               p_renderer->lighting_data.lights[0].position,
               0.1f,
               -3000,
               3000);
    ColorEdit3("Color", p_renderer->lighting_data.lights[0].color);
    DragFloat("Light Strength",
              &p_renderer->lighting_data.lights[0].strength,
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
    Text("Shadowmap");
    DragFloat(
      "Near Plane", &p_renderer->shadowmapOptions.nearPlane, 0.1f, -100, 100);
    DragFloat(
      "Far Plane", &p_renderer->shadowmapOptions.farPlane, 0.1f, -100, 100);
    DragFloat(
      "Projection Size", &p_renderer->shadowmapOptions.camSize, 0.1f, 0, 100);
    DragInt("PCF Samples", &p_renderer->shadowmapOptions.pcfSamples, 1, 0, 10);

    DragFloat("Normal Bias min",
              &p_renderer->shadowmapOptions.normalBiasMin,
              0.0001f,
              0,
              2,
              "%.5f");
    DragFloat("Normal Bias max",
              &p_renderer->shadowmapOptions.normalBiasMax,
              0.0001f,
              0,
              2,
              "%.5f");

    if (CollapsingHeader("[Texture] Shadowmap"))
    {
        Image((void *)(intptr_t)shadowmap_getTexture(),
              ImVec2(200, 200),
              ImVec2(0, 1),
              ImVec2(1, 0));
    }
}