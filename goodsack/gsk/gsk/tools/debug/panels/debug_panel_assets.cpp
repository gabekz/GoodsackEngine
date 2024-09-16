/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_assets.hpp"
#include <imgui.h>

void
gsk::tools::panels::Assets::draw(void)
{
    using namespace ImGui;

    if (CollapsingHeader("Scripts"))
    {

        Text("Lua status: ");
        BeginDisabled();
        bool luaStateEnable = true;
        bool luaStateReload = false;
        Checkbox("Running", &luaStateEnable);
        Checkbox("Auto Reload", &luaStateEnable);
        EndDisabled();
        SameLine();
        if (Button("Force Reload")) { LOG_DEBUG("Force Reload"); }
    }

    if (CollapsingHeader("Shaders"))
    {

        bool shaderReload = false;
        BeginDisabled();
        Checkbox("Auto Reload", &shaderReload);
        EndDisabled();
        SameLine();
        if (Button("Force Reload")) { LOG_DEBUG("Force Reload"); }
    }
}