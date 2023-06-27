#include "debugui.hpp"

#include <stdio.h>

#include <iostream>
#include <string>

#include <entity/v1/ecs.h>

extern "C" {
#include <core/graphics/renderer/v1/renderer.h>
}
#include <core/graphics/renderer/pipeline/pass_shadowmap.h>

#include <core/device/device.h>
#include <core/drivers/alsoft/alsoft_debug.h>

#include <entity/v1/builtin/component_test.h>
#include <entity/v1/builtin/components.h>

#include <core/drivers/vulkan/vulkan.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_vulkan.h"

DebugGui::DebugGui(Renderer *renderer)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Setup Platform/Renderer bindings
    if (DEVICE_API_OPENGL) {
        ImGui_ImplGlfw_InitForOpenGL(renderer->window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    } else if (DEVICE_API_VULKAN) {
        ImGui_ImplGlfw_InitForVulkan(renderer->window, true);

        VulkanDeviceContext *vkDevice  = renderer->vulkanDevice;
        ImGui_ImplVulkan_InitInfo info = {
          .Instance       = vkDevice->vulkanInstance,
          .PhysicalDevice = vkDevice->physicalDevice,
          .Device         = vkDevice->device,
          .Queue          = vkDevice->graphicsQueue,
          .DescriptorPool = vkDevice->descriptorPool,
          //.RenderPass = NULL,
          .MinImageCount = 2,
          .ImageCount    = 2,
          //.MsaaSamples = VK_SAMPLE_COUNT_1_BIT,
        };
        ImGui_ImplVulkan_Init(&info, vkDevice->pipelineDetails->renderPass);

        VkCommandBuffer commandBuffer =
          vulkan_command_stc_begin(vkDevice->device, vkDevice->commandPool);

        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

        vulkan_command_stc_end(vkDevice->device,
                               vkDevice->graphicsQueue,
                               commandBuffer,
                               vkDevice->commandPool);

        vkDeviceWaitIdle(vkDevice->device);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // SetStyle();

    m_renderer            = renderer;
    m_showEntityViewer    = false;
    m_showComponentViewer = false;
    m_showSceneViewer     = false;
    m_showSceneLighting   = false;
    m_showExample         = false;
    m_showAssets          = false;
    m_showProfiler        = false;

    SetVisibility(true);
}

DebugGui::~DebugGui()
{
    if (DEVICE_API_OPENGL) {
        ImGui_ImplOpenGL3_Shutdown();
    } else if (DEVICE_API_VULKAN) {
        ImGui_ImplVulkan_Shutdown();
    }
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void
DebugGui::SetVisibility(bool value)
{
    m_debugEnabled = value;
}
void
DebugGui::ToggleVisibility()
{
    m_debugEnabled = !m_debugEnabled;
}

void
DebugGui::Update()
{
    if (glfwGetKey(m_renderer->window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
        ToggleVisibility();
    }
}

void
DebugGui::Render()
{
    // Create new frame
    if (DEVICE_API_OPENGL) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    } else if (DEVICE_API_VULKAN) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    // ImGui::GetIO().FontGlobalScale = 1.2f;
    if (!m_debugEnabled) return;

    // Draw Navbar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("ImGui Demo")) { m_showExample = true; }
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(m_renderer->window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Change Scene")) {
                m_showSceneViewer = true;
                m_sceneQueued     = 0;
            }
            if (ImGui::MenuItem("Lighting")) { m_showSceneLighting = true; }
            ImGui::Separator();
            if (ImGui::MenuItem("Entities")) { m_showEntityViewer = true; }
            if (ImGui::MenuItem("Systems")) { m_showEntityViewer = true; }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Pipeline")) {
            if (ImGui::MenuItem("Assets")) { m_showAssets = true; }
            if (ImGui::MenuItem("Graphics")) { m_showProfiler = true; }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // Draw Panels
    if (m_showExample) { ImGui::ShowDemoWindow(&m_showExample); }
    if (m_showSceneViewer) {
        ImGui::BeginGroup();
        ImGui::Begin("Change Scene", &m_showSceneViewer);

        ImGui::Text("Total Scene Count: %d", m_renderer->sceneC);
        ImGui::Separator();
        ImGui::Text("Active Scene (index): %d", m_renderer->activeScene);

        ImGui::PushButtonRepeat(true);
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { m_sceneQueued--; }
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { m_sceneQueued++; }
        ImGui::PopButtonRepeat();
        ImGui::SameLine();
        ImGui::Text("index %d", m_sceneQueued);

        ImGui::SameLine();
        if (ImGui::Button("Load Scene")) {
            if (m_sceneQueued > m_renderer->sceneC || m_sceneQueued < 0) {
                m_sceneQueued = 0;
            } else {
                renderer_active_scene(m_renderer, m_sceneQueued);
                renderer_start(m_renderer);
                m_sceneQueued = m_renderer->activeScene;
            }
        }
        ImGui::End();
        ImGui::EndGroup();
    }
    if (m_showSceneLighting) {
        ImGui::BeginGroup();
        ImGui::Begin("Lighting", &m_showSceneLighting);

        ImGui::Separator();
        ImGui::Text("Directional Light");
        ImGui::DragFloat3(
          "Position", m_renderer->light->position, 0.1f, -3000, 3000);
        ImGui::ColorEdit3("Color", m_renderer->light->color);
        ImGui::DragFloat(
          "Light Strength", &m_renderer->light->strength, 1, 0, 100);

        ImGui::Separator();
        ImGui::Text("Shadowmap");
        ImGui::DragFloat(
          "Near Plane", &m_renderer->shadowmapOptions.nearPlane, 0.1f, 0, 20);
        ImGui::DragFloat(
          "Far Plane", &m_renderer->shadowmapOptions.farPlane, 0.1f, 0, 20);
        ImGui::DragFloat("Projection Size",
                         &m_renderer->shadowmapOptions.camSize,
                         0.1f,
                         0,
                         20);
        ImGui::DragInt(
          "PCF Samples", &m_renderer->shadowmapOptions.pcfSamples, 1, 0, 20);

        ImGui::DragFloat("Normal Bias min",
                         &m_renderer->shadowmapOptions.normalBiasMin,
                         0.0001f,
                         0,
                         2,
                         "%.5f");
        ImGui::DragFloat("Normal Bias max",
                         &m_renderer->shadowmapOptions.normalBiasMax,
                         0.0001f,
                         0,
                         2,
                         "%.5f");

        if (ImGui::CollapsingHeader("[Texture] Shadowmap")) {
            ImGui::Image((void *)(intptr_t)shadowmap_getTexture(),
                         ImVec2(200, 200),
                         ImVec2(0, 1),
                         ImVec2(1, 0));
        }

        ImGui::End();
        ImGui::EndGroup();
    }
    if (m_showEntityViewer) {
        // Get current scene
        ECS *ecs = m_renderer->sceneL[m_renderer->activeScene]->ecs;

        ImGui::BeginGroup();
        ImGui::Begin("Entity Viewer", &m_showEntityViewer);

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

            ImGui::TreeNodeEx((void *)(intptr_t)i, node_flags, str.c_str(), i);
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
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

        int TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

        enum MyItemColumnID { MyItemColumnID_ID, MyItemColumnID_Name };

        if (ImGui::BeginTable("table_sorting",
                              4,
                              flags,
                              ImVec2(0.0f, TEXT_BASE_HEIGHT * 15),
                              0.0f)) {

            ImGui::TableSetupColumn("ID",
                                    ImGuiTableColumnFlags_DefaultSort |
                                      ImGuiTableColumnFlags_WidthFixed,
                                    0.0f,
                                    MyItemColumnID_ID);
            ImGui::TableSetupColumn("Name",
                                    ImGuiTableColumnFlags_WidthFixed,
                                    0.0f,
                                    MyItemColumnID_Name);
            // ImGui::TableSetupColumn("Action",   ImGuiTableColumnFlags_NoSort
            // | ImGuiTableColumnFlags_WidthFixed,   0.0f,
            // MyItemColumnID_Action); ImGui::TableSetupColumn("Quantity",
            // ImGuiTableColumnFlags_PreferSortDescending |
            // ImGuiTableColumnFlags_WidthStretch, 0.0f,
            // MyItemColumnID_Quantity);
            ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
            ImGui::TableHeadersRow();

// Sort our data if sort specs have been changed!
#if DEBUG_UI_USING_SORTING
            if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs())
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
                for (int row_n = clipper.DisplayStart;
                     row_n < clipper.DisplayEnd;
                     row_n++) {
                    // Display a data item
                    // MyItem* item = &items[row_n];
                    ImGui::PushID(row_n);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%04d", row_n);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Entity");
                    ImGui::TableNextColumn();
                    if (ImGui::SmallButton("Inspect")) {
                        Entity entity         = (Entity {.id    = (EntityId)row_n,
                                                         .index = (ui64)row_n,
                                                         .ecs   = ecs});
                        m_selectedEntity      = entity;
                        m_showComponentViewer = true;
                    }
                    ImGui::PopID();
                }

            ImGui::EndTable();
        }
#endif
        ImGui::End();
        ImGui::EndGroup();
    }

    if (m_showComponentViewer) {
        ImGui::Begin("Component Viewer", &m_showComponentViewer);

        Entity e = m_selectedEntity;

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
        ImGui::Text("entity %d", e.id);
        ImGui::PopStyleColor();

        if (ecs_has(e, C_TRANSFORM)) {
            ImGui::BeginChild(
              "Transform", ImVec2(0, ImGui::GetFontSize() * 8.0f), true);

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Transform Component");
            ImGui::PopStyleColor();
            ImGui::Separator();
            // wow, this is ridiculous..
            struct ComponentTransform &p =
              *(static_cast<struct ComponentTransform *>(
                ecs_get(e, C_TRANSFORM)));
            vec3 t = GLM_VEC3_ZERO_INIT;
            ImGui::DragFloat3("Position", p.position, 0.1f, -3000, 3000);
            // ImGui::BeginDisabled();
            ImGui::DragFloat3("Rotation", p.orientation, 0.1f, -3000, 3000);
            // ImGui::EndDisabled();
            ImGui::DragFloat3("Scale", p.scale, -1, 1);
            ImGui::Separator();
            ImGui::Text("Parent Entity");
            if (p.hasParent) {
                ImGui::Text("index: %i", ((Entity *)p.parent)->index);
                ImGui::Text("id: %i", ((Entity *)p.parent)->id);
            } else {
                ImGui::Text("None");
            }

            ImGui::EndChild();
        }
        if (ecs_has(e, C_RIGIDBODY)) {
            ImGui::BeginChild(
              "Rigidbody", ImVec2(0, ImGui::GetFontSize() * 10.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Rigidbody Component");
            ImGui::PopStyleColor();
            ImGui::Separator();

            struct ComponentRigidbody &p =
              *(static_cast<struct ComponentRigidbody *>(
                ecs_get(e, C_RIGIDBODY)));

            ImGui::DragFloat3("Gravity", p.gravity, 0.1f, -3000, 3000);
            ImGui::DragFloat3("Velocity", p.velocity, 0.1f, -3000, 3000);
            ImGui::DragFloat3("Force", p.force, 0.1f, -3000, 3000);
            ImGui::DragFloat("Mass", &p.mass, 0.45f, 0.9f);

            ImGui::EndChild();
        }
        if (ecs_has(e, C_MODEL)) {
            ImGui::BeginChild(
              "Model", ImVec2(0, ImGui::GetFontSize() * 25.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Model Component");
            ImGui::PopStyleColor();
            ImGui::Separator();
            // wow, this is ridiculous..
            struct ComponentModel &p =
              *(static_cast<struct ComponentModel *>(ecs_get(e, C_MODEL)));

            // Model information
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
            ImGui::Text("Model information");
            ImGui::PopStyleColor();
            ImGui::Separator();

            ImGui::BeginDisabled();
            ImGui::InputText("Model Path", (char *)p.modelPath, 128);
            ImGui::EndDisabled();

            // Mesh information
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
            ImGui::Text("Mesh");
            ImGui::PopStyleColor();
            ImGui::Separator();

            ImGui::Text("Meshes: %u", ((Model *)p.pModel)->meshesCount);

            bool shadowVal = true;
            ImGui::BeginDisabled();
            ImGui::Checkbox("Receive Shadows", &shadowVal);
            ImGui::Checkbox("Cast Shadows", &shadowVal);
            ImGui::EndDisabled();

            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
            ImGui::Text("Material");
            ImGui::PopStyleColor();
            ImGui::Separator();

            // ImGui::Text("Shader");
            // ImGui::InputText("Vertex Shader", (char
            // *)p.material->shaderProgram->shaderSource->shaderPath, 128);
            // ImGui::InputText("Fragment Shader", (char
            // *)p.material->shaderProgram->shaderSource->shaderFragment,
            // 128);

            if (ImGui::CollapsingHeader("Textures")) {
                if (DEVICE_API_OPENGL) {
                    int textureCount = ((Material *)p.material)->texturesCount;
                    // Display textures
                    for (int i = 0; i < textureCount; i++) {
                        ImGui::Separator();
                        ImGui::Image((void *)(intptr_t)((Material *)p.material)
                                       ->textures[i]
                                       ->id,
                                     ImVec2(200, 200),
                                     ImVec2(0, 1),
                                     ImVec2(1, 0));
                        ImGui::SameLine();
                        ImGui::Text(
                          "File Path: %s\nDimensions: %dx%d\nType: %s",
                          ((Material *)p.material)->textures[i]->filePath,
                          ((Material *)p.material)->textures[i]->width,
                          ((Material *)p.material)->textures[i]->height,
                          "");
                    }
                } // DEVICE_API_OPENGL
            }     // Textures collapsing header
            ImGui::EndChild();
        }
        if (ecs_has(e, C_CAMERA)) {
            ImGui::BeginChild(
              "Camera", ImVec2(0, ImGui::GetFontSize() * 10.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Camera Component");
            ImGui::PopStyleColor();
            ImGui::Separator();
            // wow, this is ridiculous..
            struct ComponentCamera &p =
              *(static_cast<struct ComponentCamera *>(ecs_get(e, C_CAMERA)));
            ImGui::DragFloat("FOV", &p.fov, 0.45f, 0.9f);
            ImGui::DragFloat("Speed", &p.speed, 0.01, 0, 10.0f);
            ImGui::Text("Clipping");
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("Near", &p.nearZ, 0.01, 0, 10);
            ImGui::SameLine();
            ImGui::DragFloat("Far", &p.farZ, 1, 0, 1000);
            ImGui::EndChild();
        }

        if (ecs_has(e, C_AUDIOLISTENER)) {
            ImGui::BeginChild("Audio Listener");
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Audio Listener Component");
            ImGui::PopStyleColor();
            ImGui::Separator();

            ImGui::EndChild();
        }
        if (ecs_has(e, C_AUDIOSOURCE)) {
            ImGui::BeginChild(
              "Audio Source", ImVec2(0, ImGui::GetFontSize() * 10.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Audio Source Component");
            ImGui::PopStyleColor();
            ImGui::Separator();
            // wow, this is ridiculous..
            struct ComponentAudioSource &a =
              *(static_cast<struct ComponentAudioSource *>(
                ecs_get(e, C_AUDIOSOURCE)));
            // ImGui::DragFloat("FOV", &a.volume, 0.45f, 0.9f);
            // ImGui::DragFloat("Gain", a.gain, 0.1f, -3000, 3000);
            // ImGui::DragFloat("Pitch", a.pitch, 0.1f, -3000, 3000);
            ImGui::BeginDisabled();
            // ImGui::InputText("Audio File Path", (char *)a.filePath, 128);
            ImGui::EndDisabled();

            if (ImGui::Button("Play")) { AL_CHECK(alSourcePlay(a.bufferId)); }
            ImGui::SameLine();
            if (ImGui::Button("Stop")) { AL_CHECK(alSourceStop(a.bufferId)); }
            if (ImGui::Checkbox("Looping", (bool *)&a.looping)) {
                ;
                AL_CHECK(alSourcei(a.bufferId, AL_LOOPING, a.looping));
            }
            ImGui::EndChild();
        }
        if (ecs_has(e, C_ANIMATOR)) {
            ImGui::BeginChild(
              "Animator", ImVec2(0, ImGui::GetFontSize() * 10.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Animator Component");
            ImGui::PopStyleColor();
            ImGui::Separator();

            ImGui::EndChild();
        }
        if (ecs_has(e, C_TEST)) {
            ImGui::BeginChild("Lua Test Component",
                              ImVec2(0, ImGui::GetFontSize() * 8.0f),
                              true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Lua Test Component");
            ImGui::PopStyleColor();
            ImGui::Separator();
            // wow, this is ridiculous..
            struct ComponentTest &p =
              *(static_cast<struct ComponentTest *>(ecs_get(e, C_TEST)));
            int movement_increment = p.movement_increment;
            float rotation_speed   = p.rotation_speed;
            ImGui::Text("movement_increment: %d", movement_increment);
            ImGui::Text("rotation_speed: %f", rotation_speed);

            ImGui::EndChild();
        }

        if (ecs_has(e, C_WEAPON)) {
            ImGui::BeginChild(
              "Weapon Component", ImVec2(0, ImGui::GetFontSize() * 8.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("Weapon Component");
            ImGui::PopStyleColor();
            ImGui::Separator();

            struct ComponentWeapon &p =
              *(static_cast<struct ComponentWeapon *>(ecs_get(e, C_WEAPON)));

            ImGui::DragFloat3(
              "pos_starting", p.pos_starting, 0.1f, -3000, 3000);
            ImGui::DragFloat3(
              "rot_starting", p.rot_starting, 0.1f, -3000, 3000);

            ImGui::EndChild();
        }

        ImGui::End();
    }
    if (m_showAssets) {
        ImGui::Begin("Assets Pipeline", &m_showAssets);
        ImGui::BeginGroup();

        if (ImGui::CollapsingHeader("Scripts")) {

            ImGui::Text("Lua status: ");
            ImGui::BeginDisabled();
            bool luaStateEnable = true;
            bool luaStateReload = false;
            ImGui::Checkbox("Running", &luaStateEnable);
            ImGui::Checkbox("Auto Reload", &luaStateEnable);
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Force Reload")) { LOG_DEBUG("Force Reload"); }
        }

        if (ImGui::CollapsingHeader("Shaders")) {

            bool shaderReload = false;
            ImGui::BeginDisabled();
            ImGui::Checkbox("Auto Reload", &shaderReload);
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Force Reload")) { LOG_DEBUG("Force Reload"); }
        }

        ImGui::EndGroup();
        ImGui::End();
    }
    if (m_showProfiler) {

        RendererProps *props = &m_renderer->properties;

        ImGui::Begin("Graphics Pipeline", &m_showProfiler);
        ImGui::BeginGroup();

        if (ImGui::CollapsingHeader("Performace")) {
            ImGui::Separator();
            ImGui::Text("Analytics");
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("%f FPS", device_getAnalytics().currentFps);
            ImGui::Text("%f ms", device_getAnalytics().currentMs);
            ImGui::PopStyleColor();
        }

        if (ImGui::CollapsingHeader("Settings")) {
            ImGui::Separator();
            ImGui::Text("Window");
            int vsync = device_getGraphicsSettings().swapInterval;
            ImGui::Checkbox("VSync", (bool *)&vsync);
            device_setGraphicsSettings(
              (GraphicsSettings({.swapInterval = vsync})));

            ImGui::Separator();
            ImGui::Text("Ambient Occlusion");
            ImGui::DragFloat(
              "SSAO Strength", &m_renderer->ssaoOptions.strength, 0.1f, 0, 20);
            ImGui::DragFloat(
              "Bias", &m_renderer->ssaoOptions.bias, 0.0001f, 0, 2, "%.5f");
            ImGui::DragFloat(
              "Radius", &m_renderer->ssaoOptions.radius, 0.05f, 0, 2, "%.5f");
            ImGui::DragInt(
              "Kernel Size", &m_renderer->ssaoOptions.kernelSize, 1, 1, 64);

            ImGui::Separator();
            ImGui::Text("Frame");

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
            if (ImGui::BeginCombo("Tonemapping", combo_preview_value, flags)) {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                    const bool is_selected = (item_current_idx == n);
                    if (ImGui::Selectable(items[n], is_selected)) {
                        item_current_idx  = n;
                        props->tonemapper = n;
                    }

                    // Set the initial focus when opening the combo (scrolling +
                    // keyboard navigation focus)
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (item_current_idx == 2) { // Reinhard Extended
                ImGui::SliderFloat("Max White", &props->maxWhite, 0.0f, 20.0f);
            }

            ImGui::DragFloat(
              "Exposure", &props->exposure, 0.1f, 0.0f, 20.0f, "%.1f");

            ImGui::Checkbox("Gamma Correction", (bool *)&props->gammaEnable);
            if (!props->gammaEnable) ImGui::BeginDisabled();
            ImGui::DragFloat("Gamma", &props->gamma, 0.1f, 0.0f, 20.0f, "%.1f");
            if (!props->gammaEnable) ImGui::EndDisabled();

            ImGui::Separator();
            ImGui::Text("Vignette");
            ImGui::DragFloat(
              "Amount", &props->vignetteAmount, 0.1f, 0.0f, 2.0f, "%.01f");
            ImGui::DragFloat(
              "Falloff", &props->vignetteFalloff, 0.1f, 0.0f, 1.0f, "%.01f");

            ImGui::Separator();
            ImGui::Text("Anti Aliasing");
            ImGui::Checkbox("MSAA", (bool *)&props->msaaEnable);
            ImGui::SameLine();
            int samples = (int)props->msaaSamples;
            ImGui::BeginDisabled();
            ImGui::DragInt("Samples", &samples, 2, 0, 4);
            ImGui::EndDisabled();

            /*
            ImGui::Separator();
            ImGui::Text("Draw Calls: ");
            ImGui::Separator();
            ImGui::Text("Total Vertices: ");
            ImGui::Text("Total Polygons: ");
            ImGui::Text("Total Faces: ");
            // ImGui::ColorEdit3("Color", vec3{0.0, 0.0, 0.0});
            */
        }

        ImGui::EndGroup();
        ImGui::End();
    }

    // Render
    ImGui::Render();

    if (DEVICE_API_OPENGL) {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    } else if (DEVICE_API_VULKAN) {
#if 1
        VulkanDeviceContext *vkDevice = m_renderer->vulkanDevice;
        ImGui_ImplVulkan_RenderDrawData(
          ImGui::GetDrawData(),
          vkDevice->commandBuffers[vkDevice->currentFrame]);
#endif
    }
}
