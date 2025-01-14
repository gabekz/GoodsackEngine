/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_entity_viewer.hpp"

#include "entity/ecs.h"
#include "tools/debug/panels/debug_panel_component_viewer.hpp"

#include <imgui.h>

using ComponentViewer = gsk::tools::panels::ComponentViewer;

void
gsk::tools::panels::EntityViewer::set_component_viewer(ComponentViewer *ref)
{
    if (ref) { p_component_viewer = ref; }
}

void
gsk::tools::panels::EntityViewer::draw(void)
{
    using namespace ImGui;

    // Get current scene
    gsk_ECS *ecs = p_renderer->sceneL[p_renderer->activeScene]->ecs;

#if 0
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                        ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                        ImGuiTreeNodeFlags_SpanAvailWidth;

        static int node_clicked =
          -1; // must be static for persistence (TODO: Fix)
        // Go through each entity
        for (int i = 0; i < ecs->nextIndex; i++) {
            ImGuiTreeNodeFlags node_flags =
              ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
              base_flags;

            if (i == node_clicked) node_flags |= ImGuiTreeNodeFlags_Selected;

            // Grab entity by ID
            gsk_Entity e =
              (gsk_Entity {.id = (gsk_EntityId)i, .index = (u64)i, .ecs = ecs});

            std::string str = std::to_string(e.index) + " | ";
            str += "Entity id: " + std::to_string(e.id);

            TreeNodeEx((void *)(intptr_t)i, node_flags, str.c_str(), i);
            if (IsItemClicked() && !IsItemToggledOpen()) {
                node_clicked          = i;
                m_selectedEntity      = e;
                m_showComponentViewer = true;
            }
        }
#else

    // Options
    static ImGuiTableFlags flags =
      ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
      ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable |
      ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg |
      ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
      ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;

    int TEXT_BASE_HEIGHT = GetTextLineHeightWithSpacing();

    enum MyItemColumnID {
        MyItemColumnID_EntIndex,
        MyItemColumnID_EntID,
        MyItemColumnID_Name
    };

    if (BeginTable(
          "table_sorting", 4, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f))
    {

        TableSetupColumn("index",
                         ImGuiTableColumnFlags_DefaultSort |
                           ImGuiTableColumnFlags_WidthFixed,
                         0.0f,
                         MyItemColumnID_EntIndex);

        TableSetupColumn(
          "ID", ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_EntID);

        TableSetupColumn(
          "Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);

        TableSetupScrollFreeze(0, 1); // Make row always visible
        TableHeadersRow();

// Sort our data if sort specs have been changed!
#if DEBUG_UI_USING_SORTING
        if (ImGuiTableSortSpecs *sorts_specs = TableGetSortSpecs())
            if (sorts_specs->SpecsDirty)
            {
                MyItem::s_current_sort_specs =
                  sorts_specs; // Store in variable accessible by the sort
                               // function.
                if (items.Size > 1)
                    qsort(&items[0],
                          (size_t)items.Size,
                          sizeof(items[0]),
                          MyItem::CompareWithSortSpecs);
                MyItem::s_current_sort_specs = NULL;
                sorts_specs->SpecsDirty      = false;
            }
#endif // DEBUG_UI_USING_SORTING

        ImGuiListClipper clipper;
        clipper.Begin(ecs->nextIndex);
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd;
                 row_n++)
            {
                // TODO: This menu breaks when we have no initialized
                // entities
                if (!(ecs->ids_init[row_n] & GskEcsEntityFlag_Initialized))
                {
                    continue;
                }

                u8 is_disabled =
                  !(ecs->ids_init[row_n] & GskEcsEntityFlag_Enabled);

                if (is_disabled)
                {
                    PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                }

                // Display a data item
                // MyItem* item = &items[row_n];
                PushID(row_n);
                // TableNextRow();
                TableNextColumn();
                Text("%04d", row_n);
                TableNextColumn();
                Text("%04d", (int)ecs->ids[(int)row_n]);
                TableNextColumn();
                TextUnformatted(ecs->entity_names[(int)row_n]);
                if (is_disabled) { PopStyleColor(); }

                TableNextColumn();
                if (SmallButton("Inspect"))
                {
                    gsk_Entity entity = (gsk_Entity {
                      .id = ecs->ids[row_n], .index = (u64)row_n, .ecs = ecs});

                    // display the component viewer panel
                    p_component_viewer->show_for_entity(entity);
                }
                PopID();
            }

        EndTable();
    }
#endif
}