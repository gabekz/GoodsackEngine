/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_renderer.hpp"

#include "core/device/device.h"
#include <imgui.h>

#include "core/graphics/renderer/pipeline/pass_bloom.h"
#include "core/graphics/renderer/pipeline/pass_prepass.h"
#include "core/graphics/renderer/pipeline/pass_screen.h"
#include "core/graphics/renderer/pipeline/pass_ssao.h"

void
gsk::tools::panels::RenderInfo::draw(void)
{
    using namespace ImGui;
    gsk_RendererProps *props = &p_renderer->properties;

#if 1
    if (CollapsingHeader("Renderer Debug"))
    {
        Separator();
        Text("Window Info");

        {
            ivec2 res = {(int)p_renderer->windowWidth,
                         (int)p_renderer->windowHeight};
            InputInt2(
              "Window Resolution", (int *)res, ImGuiInputTextFlags_ReadOnly);

            Text("Aspect Ratio: %f", p_renderer->window_aspect_ratio);
        }
        {
            Separator();
            ivec2 res = {(int)p_renderer->renderWidth,
                         (int)p_renderer->renderHeight};
            InputInt2(
              "Render Resolution", (int *)res, ImGuiInputTextFlags_ReadOnly);
        }

        Separator();
        Text("RenderPass Info");

        if (CollapsingHeader("G-Buffer"))
        {
            Image((void *)(intptr_t)prepass_getPosition(),
                  ImVec2(320, 180),
                  ImVec2(0, 1),
                  ImVec2(1, 0));
            SameLine();
            Image((void *)(intptr_t)prepass_getNormal(),
                  ImVec2(320, 180),
                  ImVec2(0, 1),
                  ImVec2(1, 0));
        }

        if (CollapsingHeader("Shadowmap"))
        {
            Image((void *)(intptr_t)shadowmap_getTexture(),
                  ImVec2(320, 180),
                  ImVec2(0, 1),
                  ImVec2(1, 0));
        }

        if (CollapsingHeader("Bloom"))
        {
            Image((void *)(intptr_t)pass_bloom_get_texture_id(),
                  ImVec2(320, 180),
                  ImVec2(0, 1),
                  ImVec2(1, 0));
        }

        if (CollapsingHeader("SSAO Pass"))
        {
            Image((void *)(intptr_t)pass_ssao_getNoiseTextureId(),
                  ImVec2(128, 128),
                  ImVec2(0, 1),
                  ImVec2(1, 0));
            Image((void *)(intptr_t)pass_ssao_getFirstTextureId(),
                  ImVec2(320, 180),
                  ImVec2(0, 1),
                  ImVec2(1, 0));
            SameLine();
            Image((void *)(intptr_t)pass_ssao_getOutputTextureId(),
                  ImVec2(320, 180),
                  ImVec2(0, 1),
                  ImVec2(1, 0));
        }

        if (CollapsingHeader("Lighting Pass"))
        {
            Image((void *)(intptr_t)postbuffer_get_id(),
                  ImVec2(320, 180),
                  ImVec2(0, 1),
                  ImVec2(1, 0));
        }
    }
#endif
}