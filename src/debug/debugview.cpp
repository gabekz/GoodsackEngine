#include "debugview.hpp"

#include <stdio.h>

#include <iostream>
#include <string>

#include <ecs/ecs.h>

extern "C" {
    #include <core/renderer/renderer.h>
}

#include <components/transform/transform.h>
#include <components/mesh/mesh.h>
#include <components/camera/camera.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

DebugGui::DebugGui(Renderer *renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(renderer->window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //SetStyle();

    m_renderer = renderer;
    m_showEntityViewer = true;
    m_showComponentViewer = false;
    m_showSceneViewer = false;
    m_showSceneLighting = false;
    m_showExample = false;
}

DebugGui::~DebugGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DebugGui::Render() {
// Create new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::GetIO().FontGlobalScale = 1.2f;

// Draw Navbar
    if(ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Window")) {
            if(ImGui::MenuItem("ImGui Demo")) {
                m_showExample = true;
            }
            if(ImGui::MenuItem("Exit")) {
               glfwSetWindowShouldClose(m_renderer->window, GLFW_TRUE);
            }
            ImGui::EndMenu();
          }

         if (ImGui::BeginMenu("Scene")) {
            if(ImGui::MenuItem("Change Scene")) {
                m_showSceneViewer = true;
                m_sceneQueued = 0;
            }
            if(ImGui::MenuItem("Lighting")) {
                m_showSceneLighting = true;
            }
            if(ImGui::MenuItem("Entities")) {
                m_showEntityViewer = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

// Draw Panels
    if(m_showExample) {
        ImGui::ShowDemoWindow(&m_showExample);
    }
    if(m_showSceneViewer) {
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
            renderer_active_scene(m_renderer, m_sceneQueued);
            renderer_start(m_renderer);
            m_sceneQueued = m_renderer->activeScene;
        }
        ImGui::EndGroup();
    }
    if(m_showSceneLighting) {
        ImGui::BeginGroup();
        ImGui::Begin("Lighting", &m_showSceneLighting);

        ImGui::Text("Directional Light");
        //ImGui::ColorEdit3("Color", vec3{0.0, 0.0, 0.0});

        ImGui::EndGroup();
    }
    if(m_showEntityViewer) {
        // Get current scene
        ECS *ecs = m_renderer->sceneL[m_renderer->activeScene]->ecs;
        
        ImGui::BeginGroup();
        ImGui::Begin("Entity Viewer", &m_showEntityViewer);

        ImGuiTreeNodeFlags base_flags = 
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanAvailWidth;

        static int node_clicked = -1; // must be static for persistence (TODO: Fix)
        // Go through each entity
        for(int i = 0; i < ecs->nextId-1; i++) {
            ImGuiTreeNodeFlags node_flags =
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | base_flags;

            if (i == node_clicked)
                node_flags |= ImGuiTreeNodeFlags_Selected;

            // Grab entity by ID
            Entity e = (Entity){
                .id = (EntityId)i, .index = (ui64)i, .ecs = ecs};

            std::string str = std::to_string(e.index) + " | ";
            str += "Entity id: " + std::to_string(e.id);
            
            ImGui::TreeNodeEx((void *)(intptr_t)i, node_flags, str.c_str(), i);
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                node_clicked = i;
                m_selectedEntity = e;
                m_showComponentViewer = true;
            }

        }
        ImGui::EndGroup();
        ImGui::End();
    }

    if(m_showComponentViewer) {
        ImGui::Begin("Component Viewer", &m_showComponentViewer);

        Entity e = m_selectedEntity;

        if(ecs_has(e, C_TRANSFORM)) {
            ImGui::BeginChild("Transform", ImVec2(0, ImGui::GetFontSize() * 8.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
            ImGui::Text("Transform Component");
            ImGui::PopStyleColor();
            ImGui::Separator();
            // wow, this is ridiculous..
            struct ComponentTransform &p =
                *(static_cast<struct ComponentTransform *>(
                ecs_get(e, C_TRANSFORM)
            ));
            ImGui::DragFloat3("Position", p.position, 0.1f, -3000, 3000);
            ImGui::BeginDisabled();
            ImGui::DragFloat3("Scale", p.scale, -1, 1);
            ImGui::EndDisabled();
            ImGui::EndChild();
        }
        if(ecs_has(e, C_MESH)) {
            ImGui::BeginChild("Mesh", ImVec2(0, ImGui::GetFontSize() * 8.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
            ImGui::Text("Mesh Component");
            ImGui::PopStyleColor();
            ImGui::Separator();
            // wow, this is ridiculous..
            struct ComponentMesh &p =
                *(static_cast<struct ComponentMesh*>(
                ecs_get(e, C_MESH)
            ));
            ImGui::BeginDisabled();
            ImGui::Text("Model");
            ImGui::InputText("Model Path", (char *)p.modelPath, 128);
            ImGui::EndDisabled();
            //ImGui::Text("Material");
            //ImGui::InputText("Vertex Shader", (char *)p.material->shaderProgram->shaderSource->shaderVertex, 128);
            //ImGui::InputText("Fragment Shader", (char *)p.material->shaderProgram->shaderSource->shaderFragment, 128);
            ImGui::EndChild();
        }
        if(ecs_has(e, C_CAMERA)) {
            ImGui::BeginChild("Camera", ImVec2(0, ImGui::GetFontSize() * 10.0f), true);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
            ImGui::Text("Camera Component");
            ImGui::PopStyleColor();
            ImGui::Separator();
            // wow, this is ridiculous..
            struct ComponentCamera &p =
                *(static_cast<struct ComponentCamera*>(
                ecs_get(e, C_CAMERA)
            ));
            ImGui::DragFloat("FOV", &p.fov, 0.45f, 0.9f);
            ImGui::DragFloat("Speed", &p.speed, 0.01, 0, 1.0f);
            ImGui::Text("Clipping");
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("", &p.clipping.nearZ, 0.01, 0, 10);
            ImGui::SameLine();
            ImGui::DragFloat("", &p.clipping.farZ, 1, 0, 1000);
            ImGui::EndChild();
        }

        ImGui::End();
    }

// Render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
