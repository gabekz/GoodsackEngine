/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel_component_viewer.hpp"

#include "entity/ecs.h"

#include "core/device/device.h"
#include "core/drivers/alsoft/alsoft.h"
#include "core/drivers/alsoft/alsoft_debug.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/particles/particle_system.h"

#include "entity/__generated__/components_gen.h"

#include <imgui.h>

static std::string
_component_type_name(ECSComponentType component_type)
{
    switch (component_type)
    {
    case C_ANIMATOR: return "Animator";
    case C_AUDIOLISTENER: return "Audio Listener";
    case C_AUDIOSOURCE: return "Audio Source";
    case C_BANE: return "Bane";
    case C_BONE_ATTACHMENT: return "Bone Attachment";
    case C_CAMERA: return "Camera";
    case C_CAMERALOOK: return "Camera Look";
    case C_CAMERAMOVEMENT: return "Camera Movement";
    case C_COLLIDER: return "Collider";
    case C_ENEMY: return "Enemy";
    case C_ENTITY_REFERENCE: return "Entity Reference";
    case C_HEALTH: return "Health";
    case C_LIGHT: return "Light";
    case C_MODEL: return "Model";
    case C_PARTICLE_EMITTER: return "Particle Emitter";
    case C_PLAYER_CONTROLLER: return "Player Controller";
    case C_RENDERLAYER: return "Render Layer";
    case C_RIGIDBODY: return "Rigidbody";
    case C_SWORD_CONTROLLER: return "Sword Controller";
    case C_TRANSFORM: return "Transform";
    case C_WEAPON: return "Weapon";
    case C_WEAPONSWAY: return "Weapon Sway";
    default: return "None";
    }
}

static void
_draw_component_editors(gsk_Entity e, ECSComponentType cmp_type)
{
    using namespace ImGui;

    if (cmp_type == C_TRANSFORM)
    {
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
    } else if (cmp_type == C_RIGIDBODY)
    {
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
    }

    else if (cmp_type == C_COLLIDER)
    {
        struct ComponentCollider &p = *(
          static_cast<struct ComponentCollider *>(gsk_ecs_get(e, C_COLLIDER)));

        BeginDisabled();
        Checkbox("is_colliding", (bool *)&p.isColliding);
        EndDisabled();
    }

    else if (cmp_type == C_MODEL)
    {
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
                int tex_count = ((gsk_Material *)p.material)->texturesCount;
                // Display textures
                for (int i = 0; i < tex_count; i++)
                {
                    gsk_Texture *p_texture =
                      (gsk_Texture *)((gsk_Material *)p.material)->textures[i];

                    Separator();
                    Image((void *)(intptr_t)p_texture->id,
                          ImVec2(200, 200),
                          ImVec2(0, 1),
                          ImVec2(1, 0));
                    SameLine();
                    Text("File Path: %s\nDimensions: %dx%d\nType: %s",
                         p_texture->filePath,
                         p_texture->width,
                         p_texture->height,
                         "");
                }
            } // GSK_DEVICE_API_OPENGL
        }     // Textures collapsing header
    }

    else if (cmp_type == C_PARTICLE_EMITTER)
    {
        struct ComponentParticleEmitter &p =
          *(static_cast<struct ComponentParticleEmitter *>(
            gsk_ecs_get(e, C_PARTICLE_EMITTER)));

        gsk_ParticleSystem *p_particles =
          (gsk_ParticleSystem *)p.p_particle_system;

        DragFloat(
          "Particle min life", &p_particles->min_life, 0.1f, 0.0f, 100.0f);
        DragFloat(
          "Particle max life", &p_particles->max_life, 0.1f, 0.0f, 100.0f);

        Separator();

        DragFloat(
          "Particle size min", &p_particles->size_life_min, 0.1f, 0.0f, 100.0f);
        DragFloat(
          "Particle size max", &p_particles->size_life_max, 0.1f, 0.0f, 100.0f);

        Separator();

        DragFloat(
          "Particle cutoff", &p_particles->ramp_dist, 0.1f, 0.0f, 100.0f);
        DragFloat(
          "Particle updraft", &p_particles->updraft, 0.1f, 0.0f, 100.0f);

        Separator();

        InputFloat3("Convergence Point",
                    p_particles->convergence_point_world_pos);
        DragFloat("Convergence Strength",
                  &p_particles->convergence_strength,
                  0.1f,
                  0.0f,
                  100.0f);

        Separator();
        DragFloat(
          "Particle noise min", &p_particles->noise_min, 0.1f, 0.0f, 100.0f);
        DragFloat(
          "Particle noise max", &p_particles->noise_max, 0.1f, 0.0f, 100.0f);
        DragFloat("Particle noise multiplier",
                  &p_particles->noise_multiplier,
                  0.1f,
                  0.0f,
                  100.0f);
        DragFloat("Particle noise speed",
                  &p_particles->noise_speed,
                  0.1f,
                  0.0f,
                  100.0f);

        Separator();

        SliderInt("Particle Count",
                  &p_particles->particle_count,
                  0,
                  _GSK_MAX_PARTICLE_COUNT);
    }

    else if (cmp_type == C_PLAYER_CONTROLLER)
    {
        struct ComponentPlayerController &p =
          *(static_cast<struct ComponentPlayerController *>(
            gsk_ecs_get(e, C_PLAYER_CONTROLLER)));

        DragFloat("Walk Speed", &p.speed, 0.1f, 0.0f, 100.0f);
        DragFloat("Jump Force", &p.jump_force, 1.0f, 0.0f, 1000.0f);
        BeginDisabled();
        Checkbox("is_grounded", (bool *)&p.is_grounded);
        Checkbox("is_jumping", (bool *)&p.is_jumping);
        Checkbox("can_jump", (bool *)&p.can_jump);
        EndDisabled();
    }

    else if (cmp_type == C_CAMERA)
    {
        // wow, this is ridiculous..
        struct ComponentCamera &p =
          *(static_cast<struct ComponentCamera *>(gsk_ecs_get(e, C_CAMERA)));
        DragFloat("FOV", &p.fov, 0.45f, 0.9f);
        Text("Clipping");
        PushItemWidth(100);
        DragFloat("Near", &p.nearZ, 0.01, 0, 10);
        SameLine();
        DragFloat("Far", &p.farZ, 1, 0, 1000);
    }

    else if (cmp_type == C_CAMERALOOK)
    {
        struct ComponentCameraLook &p =
          *(static_cast<struct ComponentCameraLook *>(
            gsk_ecs_get(e, C_CAMERALOOK)));
        DragFloat("Sensitivity", &p.sensitivity, 0.45f, 0.9f);
    }

    else if (cmp_type == C_CAMERAMOVEMENT)
    {
        struct ComponentCameraMovement &p =
          *(static_cast<struct ComponentCameraMovement *>(
            gsk_ecs_get(e, C_CAMERAMOVEMENT)));
        DragFloat("Speed ", &p.speed, 0.45f, 0.9f);
    }

    else if (cmp_type == C_AUDIOSOURCE)
    {
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
    }

    else if (cmp_type == C_ANIMATOR)
    {

        struct ComponentAnimator &a = *(
          static_cast<struct ComponentAnimator *>(gsk_ecs_get(e, C_ANIMATOR)));

        gsk_Skeleton *p_skeleton = ((gsk_Animation *)a.cntAnimation)->pSkeleton;

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
    }

    else if (cmp_type == C_WEAPON)
    {
        struct ComponentWeapon &p =
          *(static_cast<struct ComponentWeapon *>(gsk_ecs_get(e, C_WEAPON)));

        DragFloat3("pos_starting", p.pos_starting, 0.1f, -3000, 3000);
        DragFloat3("rot_starting", p.rot_starting, 0.1f, -3000, 3000);
    }
}

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

    for (int i = 0; i < ECSCOMPONENT_LAST + 1; i++)
    {
        u32 selected_component = i;

        // NOTE: Weird hack to swap Transform and the component in it's place to
        // have Transform always show on top.
        if (i == 0)
        {
            selected_component = C_TRANSFORM;
        } else if (i == C_TRANSFORM)
        {
            selected_component = 0;
        }

        ECSComponentType cmp_type = (ECSComponentType)(selected_component);

        // skip if entity does not have component
        if (!(gsk_ecs_has(e, cmp_type))) continue;

        std::string _cmp_name = _component_type_name(cmp_type);

        if (CollapsingHeader(_cmp_name.c_str()))
        {
#if 0
            PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            Text(_cmp_name.c_str());
            PopStyleColor();

            Separator();
#endif

            // component-specific editor
            _draw_component_editors(e, cmp_type);
        }
    }
}
