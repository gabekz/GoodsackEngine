#include "debug_panel_lighting.hpp"
#include <imgui.h>

void
gsk::tools::panels::Lighting::draw(void)
{
    using namespace ImGui;

    Separator();
    Text("Directional Light");
    DragFloat3("Position", p_renderer->light->position, 0.1f, -3000, 3000);
    ColorEdit3("Color", p_renderer->light->color);
    DragFloat("Light Strength", &p_renderer->light->strength, 1, 0, 100);

    Separator();
    Text("Shadowmap");
    DragFloat(
      "Near Plane", &p_renderer->shadowmapOptions.nearPlane, 0.1f, 0, 20);
    DragFloat("Far Plane", &p_renderer->shadowmapOptions.farPlane, 0.1f, 0, 20);
    DragFloat(
      "Projection Size", &p_renderer->shadowmapOptions.camSize, 0.1f, 0, 20);
    DragInt("PCF Samples", &p_renderer->shadowmapOptions.pcfSamples, 1, 0, 20);

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

    if (CollapsingHeader("[Texture] Shadowmap")) {
        Image((void *)(intptr_t)shadowmap_getTexture(),
              ImVec2(200, 200),
              ImVec2(0, 1),
              ImVec2(1, 0));
    }
}