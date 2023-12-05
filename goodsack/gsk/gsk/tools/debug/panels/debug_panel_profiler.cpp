/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_profiler.hpp"

#include "core/device/device.h"
#include <imgui.h>

void
gsk::tools::panels::Profiler::draw(void)
{
    using namespace ImGui;
    gsk_RendererProps *props = &p_renderer->properties;

    if (CollapsingHeader("Performace")) {
        Separator();
        Text("Analytics");
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("%f FPS", gsk_device_getAnalytics().currentFps);
        Text("%f ms", gsk_device_getAnalytics().currentMs);
        PopStyleColor();
    }

    if (CollapsingHeader("Settings")) {
        Separator();
        Text("Window");
        int vsync = gsk_device_getGraphicsSettings().swapInterval;
        Checkbox("VSync", (bool *)&vsync);
        gsk_device_setGraphicsSettings(
          (gsk_GraphicsSettings({.swapInterval = vsync})));

        Separator();
        Text("Ambient Occlusion");
        DragFloat(
          "SSAO Strength", &p_renderer->ssaoOptions.strength, 0.1f, 0, 20);
        DragFloat("Bias", &p_renderer->ssaoOptions.bias, 0.0001f, 0, 2, "%.5f");
        DragFloat(
          "Radius", &p_renderer->ssaoOptions.radius, 0.05f, 0, 2, "%.5f");
        DragInt("Kernel Size", &p_renderer->ssaoOptions.kernelSize, 1, 1, 64);

        Separator();
        Text("Frame");

        static ImGuiComboFlags flags = 0;
        // Using the generic BeginCombo() API, you have full control over
        // how to display the combo contents. (your selection data could be
        // an index, a pointer to the object, an id for the object, a flag
        // intrusively stored in the object itself, etc.)
        const char *items[] = {"Reinhard",
                               "Reinhard (Jodie)",
                               "Reinhard (Extended)",
                               "ACES (Approximate)",
                               "Uncharted 2 Filmic"};
        static int item_current_idx =
          0; // Here we store our selection data as an index.
        const char *combo_preview_value =
          items[item_current_idx]; // Pass in the preview value visible
                                   // before opening the combo (it could be
                                   // anything)
        if (BeginCombo("Tonemapping", combo_preview_value, flags)) {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                const bool is_selected = (item_current_idx == n);
                if (Selectable(items[n], is_selected)) {
                    item_current_idx  = n;
                    props->tonemapper = n;
                }

                // Set the initial focus when opening the combo (scrolling +
                // keyboard navigation focus)
                if (is_selected) SetItemDefaultFocus();
            }
            EndCombo();
        }
        if (item_current_idx == 2) { // Reinhard Extended
            SliderFloat("Max White", &props->maxWhite, 0.0f, 20.0f);
        }

        DragFloat("Exposure", &props->exposure, 0.1f, 0.0f, 20.0f, "%.1f");

        Checkbox("Gamma Correction", (bool *)&props->gammaEnable);
        if (!props->gammaEnable) BeginDisabled();
        DragFloat("Gamma", &props->gamma, 0.1f, 0.0f, 20.0f, "%.1f");
        if (!props->gammaEnable) EndDisabled();

        Separator();
        Text("Vignette");
        DragFloat("Amount", &props->vignetteAmount, 0.1f, 0.0f, 2.0f, "%.01f");
        DragFloat(
          "Falloff", &props->vignetteFalloff, 0.1f, 0.0f, 1.0f, "%.01f");

        Separator();
        Text("Anti Aliasing");
        Checkbox("MSAA", (bool *)&props->msaaEnable);
        SameLine();
        int samples = (int)props->msaaSamples;
        BeginDisabled();
        DragInt("Samples", &samples, 2, 0, 4);
        EndDisabled();

        /*
        Separator();
        Text("Draw Calls: ");
        Separator();
        Text("Total Vertices: ");
        Text("Total Polygons: ");
        Text("Total Faces: ");
        // ColorEdit3("Color", vec3{0.0, 0.0, 0.0});
        */
    }
}