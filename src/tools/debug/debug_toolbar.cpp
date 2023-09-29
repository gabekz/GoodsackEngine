#include "debug_toolbar.hpp"

#include <stdio.h>

#include <iostream>
#include <string>

#include <entity/v1/ecs.h>

extern "C" {
#include <core/graphics/renderer/v1/renderer.h>
}

#include <core/device/device.h>
#include <core/drivers/vulkan/vulkan.h>

#include <tools/debug/debug_panel.hpp>
// debug panels
#include <tools/debug/panels/debug_panel_assets.hpp>
#include <tools/debug/panels/debug_panel_component_viewer.hpp>
#include <tools/debug/panels/debug_panel_entity_viewer.hpp>
#include <tools/debug/panels/debug_panel_lighting.hpp>
#include <tools/debug/panels/debug_panel_profiler.hpp>
#include <tools/debug/panels/debug_panel_scene_viewer.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_vulkan.h"

gsk::tools::DebugToolbar::DebugToolbar(Renderer *renderer)
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

    m_renderer = renderer;

    // Create menu panels
    {
        using namespace gsk::tools::panels;

        // Create EntityViewer panel, attach a new ComponentViewer pointer to
        // it.
        ComponentViewer *p_component_viewer = new ComponentViewer("Components");
        EntityViewer *p_entity_viewer = new EntityViewer("Entities");
        p_entity_viewer->set_component_viewer(p_component_viewer);

        // "File" Menu
        // NONE

        // "Scene" Menu
        add_panel((DebugPanel *)(new SceneViewer("Change Scene")),
                  (int)Menus::Scene);
        add_panel((DebugPanel *)(new Lighting("Lighting")), (int)Menus::Scene);

        add_panel(p_entity_viewer, (int)Menus::Scene);
        add_panel(p_component_viewer, (int)Menus::None); // Don't show this in the toolbar

        // "Pipeline" Menu
        add_panel((DebugPanel *)(new Assets("Assets")), (int)Menus::Pipeline);
        add_panel((DebugPanel *)(new Profiler("Graphics")),
                  (int)Menus::Pipeline);
    }

    set_visibility(true);
}

gsk::tools::DebugToolbar::~DebugToolbar()
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
gsk::tools::DebugToolbar::set_visibility(bool value)
{
    m_debugEnabled = value;
}
void
gsk::tools::DebugToolbar::toggle_visibility(void)
{
    m_debugEnabled = !m_debugEnabled;
}

void
gsk::tools::DebugToolbar::update(void)
{
    if (glfwGetKey(m_renderer->window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
        toggle_visibility();
    }
}

void
gsk::tools::DebugToolbar::add_panel(gsk::tools::DebugPanel *panel,
                                    int menu_index)
{
    // panel->set_menu_index((int)this->p_menus[menu_index]);
    panel->set_menu_index(menu_index);
    panel->p_renderer = this->m_renderer;
    debug_panels.push_back(panel);
}

void
gsk::tools::DebugToolbar::render(void)
{
    // ImGui::GetIO().FontGlobalScale = 1.2f;
    if (!m_debugEnabled) return;

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

    // Draw Toolbelt
    if (ImGui::BeginMainMenuBar()) {

        for (int i = 0; i < DEBUG_MENUS_COUNT; i++) {
            if (ImGui::BeginMenu(p_menu_names[i])) {

                // for each panel
                for (auto it = debug_panels.begin(); it != debug_panels.end();
                     ++it) {

                    if ((*it)->menu_index != i) { continue; }

                    // selected, set visibility
                    if (ImGui::MenuItem((*it)->title.c_str())) {
                        (*it)->visible = true;
                    }
                }

                if (i == (int)Menus::File) {
                    if (ImGui::MenuItem("ImGui Demo")) {
                        show_example_panel = true;
                    }
                    if (ImGui::MenuItem("Exit")) {
                        glfwSetWindowShouldClose(m_renderer->window, GLFW_TRUE);
                    }
                }

                ImGui::EndMenu();
            }
        }

        ImGui::EndMainMenuBar();
    }

    // Draw panels
    for (auto it = debug_panels.begin(); it != debug_panels.end(); ++it) {
        DebugPanel *panel = (*it);
        if (panel->visible) {
            ImGui::BeginGroup();
            ImGui::Begin(panel->title.c_str(), &(panel->visible));

            panel->draw();

            ImGui::End();
            ImGui::EndGroup();
        }
    }

    // ImGui Demo Panel
    if (show_example_panel) { ImGui::ShowDemoWindow(&show_example_panel); }

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
