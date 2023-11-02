#include "debug_panel_entity_viewer.hpp"

#include <entity/v1/ecs.h>
#include <imgui.h>

#include <tools/debug/panels/debug_panel_component_viewer.hpp>

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
    ECS *ecs = p_renderer->sceneL[p_renderer->activeScene]->ecs;

#if 0
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                        ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                        ImGuiTreeNodeFlags_SpanAvailWidth;

        static int node_clicked =
          -1; // must be static for persistence (TODO: Fix)
        // Go through each entity
        for (int i = 0; i < ecs->nextId - 1; i++) {
            ImGuiTreeNodeFlags node_flags =
              ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
              base_flags;

            if (i == node_clicked) node_flags |= ImGuiTreeNodeFlags_Selected;

            // Grab entity by ID
            Entity e =
              (Entity {.id = (EntityId)i, .index = (ui64)i, .ecs = ecs});

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

    enum MyItemColumnID { MyItemColumnID_Index, MyItemColumnID_Name };

    if (BeginTable("table_sorting",
                   4,
                   flags,
                   ImVec2(0.0f, TEXT_BASE_HEIGHT * 15),
                   0.0f)) {

        TableSetupColumn("index",
                         ImGuiTableColumnFlags_DefaultSort |
                           ImGuiTableColumnFlags_WidthFixed,
                         0.0f,
                         MyItemColumnID_Index);
        TableSetupColumn(
          "Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
        // TableSetupColumn("Action",   ImGuiTableColumnFlags_NoSort
        // | ImGuiTableColumnFlags_WidthFixed,   0.0f,
        // MyItemColumnID_Action); TableSetupColumn("Quantity",
        // ImGuiTableColumnFlags_PreferSortDescending |
        // ImGuiTableColumnFlags_WidthStretch, 0.0f,
        // MyItemColumnID_Quantity);
        TableSetupScrollFreeze(0, 1); // Make row always visible
        TableHeadersRow();

// Sort our data if sort specs have been changed!
#if DEBUG_UI_USING_SORTING
        if (ImGuiTableSortSpecs *sorts_specs = TableGetSortSpecs())
            if (sorts_specs->SpecsDirty) {
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
        clipper.Begin(ecs->nextId - 1);
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd;
                 row_n++) {
                // Display a data item
                // MyItem* item = &items[row_n];
                PushID(row_n);
                TableNextRow();
                TableNextColumn();
                Text("%04d", row_n);
                TableNextColumn();
                TextUnformatted("Entity");
                TableNextColumn();
                if (SmallButton("Inspect")) {
                    Entity entity = (Entity {.id    = (EntityId)row_n + 1,
                                             .index = (ui64)row_n,
                                             .ecs   = ecs});

                    // display the component viewer panel
                    p_component_viewer->show_for_entity(entity);
                }
                PopID();
            }

        EndTable();
    }
#endif
}