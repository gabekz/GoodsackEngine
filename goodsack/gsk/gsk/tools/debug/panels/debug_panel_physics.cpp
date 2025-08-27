/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_physics.hpp"

#include "core/device/device.h"
#include "util/sysdefs.h"

#include <imgui.h>

void
gsk::tools::panels::Physics::draw(void)
{
    using namespace ImGui;

    gsk_DebugPhysicsOptions *physics_options =
      &p_renderer->debugContext->physics_options;

    Checkbox("Selected Entity Only",
             (bool *)&physics_options->selected_entity_only);
    Checkbox("Draw Collisions", (bool *)&physics_options->draw_collisions);
    Checkbox("Draw Friction Debug", (bool *)&physics_options->draw_friction);

    if (CollapsingHeader("Performace"))
    {
        Separator();
        Text("Analytics");
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        PopStyleColor();
    }
}