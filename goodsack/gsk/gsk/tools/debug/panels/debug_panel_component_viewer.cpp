/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_component_viewer.hpp"

#include "entity/ecs.h"

#include "core/device/device.h"
#include "core/drivers/alsoft/alsoft.h"
#include "core/drivers/alsoft/alsoft_debug.h"
#include "core/graphics/mesh/model.h"

#include <imgui.h>

void
gsk::tools::panels::ComponentViewer::show_for_entity(gsk_Entity entity)
{
    selected_entity = entity;
    visible         = true;
}

void
gsk::tools::panels::ComponentViewer::draw(void)
{
    using namespace ImGui;

    gsk_Entity e = this->selected_entity;

    PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    Text("entity %d", e.id);
    PopStyleColor();

    if (gsk_ecs_has(e, C_TRANSFORM))
    {
        BeginChild("Transform", ImVec2(0, GetFontSize() * 12.0f), true);

        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Transform Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentTransform &p =
          *(static_cast<struct ComponentTransform *>(
            gsk_ecs_get(e, C_TRANSFORM)));
        vec3 t = GLM_VEC3_ZERO_INIT;
        DragFloat3("Position", p.position, 0.1f, -3000, 3000);
        // BeginDisabled();
        DragFloat3("Rotation", p.orientation, 0.1f, -3000, 3000);
        // EndDisabled();
        DragFloat3("Scale", p.scale, -1, 1);
        Separator();
        Text("Parent Entity");
        if (p.hasParent)
        {
            Text("index: %i", ((gsk_Entity *)p.parent)->index);
            Text("id: %i", ((gsk_Entity *)p.parent)->id);
        } else
        {
            Text("None");
        }

        EndChild();
    }
    if (gsk_ecs_has(e, C_RIGIDBODY))
    {
        BeginChild("Rigidbody", ImVec2(0, GetFontSize() * 14.0f), true);

        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Rigidbody Component");
        PopStyleColor();

        Separator();

        struct ComponentRigidbody &p =
          *(static_cast<struct ComponentRigidbody *>(
            gsk_ecs_get(e, C_RIGIDBODY)));

        DragFloat3("Gravity", p.gravity, 0.1f, -3000, 3000);
        DragFloat3("Force", p.force, 0.1f, -3000, 3000);
        DragFloat("Mass", &p.mass, 0.45f, 0.1f, 0.1f);

        Separator();

        DragFloat3("Linear Velocity", p.linear_velocity, 0.1f, -3000, 3000);
        DragFloat3("Angular Velocity", p.angular_velocity, 0.1f, -3000, 3000);

        Separator();

        DragFloat("Static Friction", &p.static_friction, 0.1f, 0.0f, 1.0f);
        DragFloat("Dynamic Friction", &p.dynamic_friction, 0.1f, 0.0f, 1.0f);

        EndChild();
    }
    if (gsk_ecs_has(e, C_COLLIDER))
    {
        BeginChild("Collider", ImVec2(0, GetFontSize() * 14.0f), true);

        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Collider Component");
        PopStyleColor();

        Separator();

        struct ComponentCollider &p = *(
          static_cast<struct ComponentCollider *>(gsk_ecs_get(e, C_COLLIDER)));

        EndChild();
    }
    if (gsk_ecs_has(e, C_MODEL))
    {
        BeginChild("Model", ImVec2(0, GetFontSize() * 15.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Model Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentModel &p =
          *(static_cast<struct ComponentModel *>(gsk_ecs_get(e, C_MODEL)));

        // Model information
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
        Text("Model information");
        PopStyleColor();
        Separator();

        BeginDisabled();
        InputText("Model Path", (char *)p.modelPath, 128);
        EndDisabled();

        // Mesh information
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
        Text("Mesh");
        PopStyleColor();
        Separator();

        Text("Meshes: %u", ((gsk_Model *)p.pModel)->meshesCount);

        bool shadowVal = true;
        BeginDisabled();
        Checkbox("Receive Shadows", &shadowVal);
        Checkbox("Cast Shadows", &shadowVal);
        EndDisabled();

        Separator();
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
        Text("Material");
        PopStyleColor();
        Separator();

        // Text("Shader");
        // InputText("Vertex Shader", (char
        // *)p.material->shaderProgram->shaderSource->shaderPath, 128);
        // InputText("Fragment Shader", (char
        // *)p.material->shaderProgram->shaderSource->shaderFragment,
        // 128);

        if (CollapsingHeader("Textures"))
        {
            if (GSK_DEVICE_API_OPENGL)
            {
                int textureCount = ((gsk_Material *)p.material)->texturesCount;
                // Display textures
                for (int i = 0; i < textureCount; i++)
                {
                    Separator();
                    Image((void *)(intptr_t)((gsk_Material *)p.material)
                            ->textures[i]
                            ->id,
                          ImVec2(200, 200),
                          ImVec2(0, 1),
                          ImVec2(1, 0));
                    SameLine();
                    Text("File Path: %s\nDimensions: %dx%d\nType: %s",
                         ((gsk_Material *)p.material)->textures[i]->filePath,
                         ((gsk_Material *)p.material)->textures[i]->width,
                         ((gsk_Material *)p.material)->textures[i]->height,
                         "");
                }
            } // GSK_DEVICE_API_OPENGL
        }     // Textures collapsing header
        EndChild();
    }
    if (gsk_ecs_has(e, C_CAMERA))
    {
        BeginChild("Camera", ImVec2(0, GetFontSize() * 10.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Camera Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentCamera &p =
          *(static_cast<struct ComponentCamera *>(gsk_ecs_get(e, C_CAMERA)));
        DragFloat("FOV", &p.fov, 0.45f, 0.9f);
        Text("Clipping");
        PushItemWidth(100);
        DragFloat("Near", &p.nearZ, 0.01, 0, 10);
        SameLine();
        DragFloat("Far", &p.farZ, 1, 0, 1000);
        EndChild();
    }
    if (gsk_ecs_has(e, C_CAMERALOOK))
    {
        BeginChild("Camera Look", ImVec2(0, GetFontSize() * 6.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("CameraLook Component");
        PopStyleColor();
        Separator();
        struct ComponentCameraLook &p =
          *(static_cast<struct ComponentCameraLook *>(
            gsk_ecs_get(e, C_CAMERALOOK)));
        DragFloat("Sensitivity", &p.sensitivity, 0.45f, 0.9f);
        EndChild();
    }

    if (gsk_ecs_has(e, C_CAMERAMOVEMENT))
    {
        BeginChild("Camera Movement", ImVec2(0, GetFontSize() * 6.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("CameraMovement Component");
        PopStyleColor();
        Separator();
        struct ComponentCameraMovement &p =
          *(static_cast<struct ComponentCameraMovement *>(
            gsk_ecs_get(e, C_CAMERAMOVEMENT)));
        DragFloat("Speed ", &p.speed, 0.45f, 0.9f);
        EndChild();
    }

    if (gsk_ecs_has(e, C_AUDIOLISTENER))
    {
        BeginChild("Audio Listener");
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Audio Listener Component");
        PopStyleColor();
        Separator();

        EndChild();
    }
    if (gsk_ecs_has(e, C_AUDIOSOURCE))
    {
        BeginChild("Audio Source", ImVec2(0, GetFontSize() * 10.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Audio Source Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentAudioSource &a =
          *(static_cast<struct ComponentAudioSource *>(
            gsk_ecs_get(e, C_AUDIOSOURCE)));
        // DragFloat("FOV", &a.volume, 0.45f, 0.9f);
        // DragFloat("Gain", a.gain, 0.1f, -3000, 3000);
        // DragFloat("Pitch", a.pitch, 0.1f, -3000, 3000);
        BeginDisabled();
        // InputText("Audio File Path", (char *)a.filePath, 128);
        EndDisabled();

        if (Button("Play")) { AL_CHECK(alSourcePlay(a.buffer_source)); }
        SameLine();
        if (Button("Stop")) { AL_CHECK(alSourceStop(a.buffer_source)); }
        if (Checkbox("Looping", (bool *)&a.is_looping))
        {
            AL_CHECK(alSourcei(a.buffer_source, AL_LOOPING, a.is_looping));
        }
        EndChild();
    }
    if (gsk_ecs_has(e, C_ANIMATOR))
    {

        struct ComponentAnimator &a = *(
          static_cast<struct ComponentAnimator *>(gsk_ecs_get(e, C_ANIMATOR)));

        gsk_Skeleton *p_skeleton = ((gsk_Animation *)a.cntAnimation)->pSkeleton;

        BeginChild("Animator", ImVec2(0, GetFontSize() * 10.0f), true);

        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Animator Component");
        PopStyleColor();

        Separator();

        Text("Total Animations: %d",
             ((gsk_Animation *)a.cntAnimation)->pSkeleton->animations_count);

        Text("Keyframes: %d",
             ((gsk_Animation *)a.cntAnimation)->keyframesCount);

        Separator();

        Checkbox("Transition Delay", (bool *)&a.is_transition_delayed);
        Checkbox("Loop", (bool *)&a.is_looping);

        BeginDisabled();
        Checkbox("Is Playing", (bool *)&a.is_playing);
        EndDisabled();

        BeginDisabled((bool *)a.is_playing);
        if (Button("Play")) { a.is_playing = 1; }
        EndDisabled();
        SameLine();
        BeginDisabled(!(bool *)a.is_playing && !(bool *)a.is_looping);
        if (Button("Stop")) { a.is_looping = 0; }
        SameLine();
        if (Button("Force Stop"))
        {
            a.is_looping = 0;
            a.is_playing = 0;
        }
        EndDisabled();

        int item_current_idx =
          (int)p_skeleton->cnt_animation_index; // selection
        const char *combo_preview_value =
          p_skeleton->p_animations[item_current_idx]->name;

        if (ImGui::BeginCombo("Animations", combo_preview_value))
        {
            for (int i = 0; i < p_skeleton->animations_count; i++)
            {
                const bool is_selected = (item_current_idx == i);
                if (ImGui::Selectable(p_skeleton->p_animations[i]->name,
                                      is_selected))
                {
                    item_current_idx = i;
                    gsk_skeleton_set_animation(p_skeleton, i);
                }

                // Set the initial focus when opening the combo (scrolling +
                // keyboard navigation focus)
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        EndChild();
    }

    if (gsk_ecs_has(e, C_WEAPON))
    {
        BeginChild("Weapon Component", ImVec2(0, GetFontSize() * 8.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Weapon Component");
        PopStyleColor();
        Separator();

        struct ComponentWeapon &p =
          *(static_cast<struct ComponentWeapon *>(gsk_ecs_get(e, C_WEAPON)));

        DragFloat3("pos_starting", p.pos_starting, 0.1f, -3000, 3000);
        DragFloat3("rot_starting", p.rot_starting, 0.1f, -3000, 3000);

        EndChild();
    }
}
