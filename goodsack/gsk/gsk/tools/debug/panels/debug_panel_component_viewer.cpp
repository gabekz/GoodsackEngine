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

#include <runtime/gsk_runtime.hpp>

#include "entity/__generated__/components_gen.h"

#include <imgui.h>

static std::string
_component_type_name(ECSComponentType component_type)
{
    switch (component_type)
    {
    case C_ANIMATOR: return "Animator";
    case C_AUDIO_LISTENER: return "Audio Listener";
    case C_AUDIO_SOURCE: return "Audio Source";
    case C_BANE: return "Bane";
    case C_BONE_ATTACHMENT: return "Bone Attachment";
    case C_CAMERA: return "Camera";
    case C_CAMERA_LOOK: return "Camera Look";
    case C_CAMERA_MOVEMENT: return "Camera Movement";
    case C_COLLIDER: return "Collider";
    case C_ENEMY: return "Enemy";
    case C_ENTITY_REFERENCE: return "Entity Reference";
    case C_FLAMMABLE: return "Flammable";
    case C_HEALTH: return "Health";
    case C_LIGHT: return "Light";
    case C_MODEL: return "Model";
    case C_PARTICLE_EMITTER: return "Particle Emitter";
    case C_PLAYER_CONTROLLER: return "Player Controller";
    case C_RENDER_LAYER: return "Render Layer";
    case C_RIGIDBODY: return "Rigidbody";
    case C_SWORD_CONTROLLER: return "Sword Controller";
    case C_TRANSFORM: return "Transform";
    case C_WEAPON: return "Weapon";
    case C_WEAPON_SWAY: return "Weapon Sway";
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
        DragFloat3("Local Position", p.position, 0.1f, -3000, 3000);
        BeginDisabled();
        DragFloat3("World Position", p.world_position, 0.1f, -3000, 3000);
        EndDisabled();
        // BeginDisabled();
        DragFloat3("Rotation", p.orientation, 0.1f, -3000, 3000);
        // EndDisabled();
        DragFloat3("Scale", p.scale, -1, 1);
        Separator();
        Text("Parent Entity");
        if (p.has_parent)
        {
            Text("id: %i", p.parent_entity_id);
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
        DragFloat3("Force", p.force_velocity, 0.1f, -3000, 3000);
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
        Checkbox("is_trigger", (bool *)&p.is_trigger);
        EndDisabled();
    }

    else if (cmp_type == C_FLAMMABLE)
    {
        struct ComponentFlammable &p =
          *(static_cast<struct ComponentFlammable *>(
            gsk_ecs_get(e, C_FLAMMABLE)));

        DragFloat("current heat", &p.current_heat);
        DragFloat("ignition point", &p.ignition_point);
        DragFloat("max heat", &p.max_heat);
        DragFloat("cooldown speed", &p.cooldown_speed);
        BeginDisabled();
        Checkbox("is_burning", (bool *)&p.is_burning);
        EndDisabled();
    }

    else if (cmp_type == C_HEALTH)
    {
        struct ComponentHealth &p =
          *(static_cast<struct ComponentHealth *>(gsk_ecs_get(e, C_HEALTH)));

        DragInt("current health", &p.current_health);
        DragInt("max health", &p.max_health);
        BeginDisabled();
        DragInt("last health", &p.last_health);
        Checkbox("is_alive", (bool *)&p.is_alive);
        Checkbox("event_health_change", (bool *)&p.event_health_change);
        EndDisabled();
    }

    else if (cmp_type == C_LIGHT)
    {
        struct ComponentLight &p =
          *(static_cast<struct ComponentLight *>(gsk_ecs_get(e, C_LIGHT)));

        ColorEdit3("Point Light Color", p.color);
        DragFloat("Point Light Intensity", &p.intensity);

    }

    else if (cmp_type == C_MODEL)
    {
        // wow, this is ridiculous..
        struct ComponentModel &p =
          *(static_cast<struct ComponentModel *>(gsk_ecs_get(e, C_MODEL)));
        gsk_Model *p_model = (gsk_Model *)p.pModel;

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

        Separator();

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

        Checkbox("System is_awake", (bool *)&p.is_awake);

        DragFloat("Particle min life",
                  &p_particles->settings.min_life,
                  0.1f,
                  0.0f,
                  100.0f);
        DragFloat("Particle max life",
                  &p_particles->settings.max_life,
                  0.1f,
                  0.0f,
                  100.0f);

        Separator();

        DragFloat("Particle size min",
                  &p_particles->settings.size_life_min,
                  0.1f,
                  0.0f,
                  100.0f);
        DragFloat("Particle size max",
                  &p_particles->settings.size_life_max,
                  0.1f,
                  0.0f,
                  100.0f);

        Separator();

        DragFloat("Particle cutoff",
                  &p_particles->settings.ramp_dist,
                  0.1f,
                  0.0f,
                  100.0f);
        DragFloat("Particle updraft",
                  &p_particles->settings.updraft,
                  0.1f,
                  0.0f,
                  100.0f);

        Separator();

        InputFloat3("Convergence Point",
                    p_particles->settings.convergence_point_world_pos);
        DragFloat("Convergence Strength",
                  &p_particles->settings.convergence_strength,
                  0.1f,
                  0.0f,
                  100.0f);

        Separator();
        DragFloat("Particle noise min",
                  &p_particles->settings.noise_min,
                  0.1f,
                  0.0f,
                  100.0f);
        DragFloat("Particle noise max",
                  &p_particles->settings.noise_max,
                  0.1f,
                  0.0f,
                  100.0f);
        DragFloat("Particle noise multiplier",
                  &p_particles->settings.noise_multiplier,
                  0.1f,
                  0.0f,
                  100.0f);
        DragFloat("Particle noise speed",
                  &p_particles->settings.noise_speed,
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
        Separator();
        BeginDisabled();
        DragFloat2("Move Axes", p.move_axes, 0.1f, -3000, 3000);
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

    else if (cmp_type == C_CAMERA_LOOK)
    {
        struct ComponentCameraLook &p =
          *(static_cast<struct ComponentCameraLook *>(
            gsk_ecs_get(e, C_CAMERA_LOOK)));
        DragFloat("Sensitivity", &p.sensitivity, 0.45f, 0.9f);
    }

    else if (cmp_type == C_CAMERA_MOVEMENT)
    {
        struct ComponentCameraMovement &p =
          *(static_cast<struct ComponentCameraMovement *>(
            gsk_ecs_get(e, C_CAMERA_MOVEMENT)));
        DragFloat("Speed ", &p.speed, 0.45f, 0.9f);
    }

    else if (cmp_type == C_AUDIO_SOURCE)
    {
        // wow, this is ridiculous..
        struct ComponentAudioSource &a =
          *(static_cast<struct ComponentAudioSource *>(
            gsk_ecs_get(e, C_AUDIO_SOURCE)));
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
        // TODO: FIX THIS SHIT BITCH

        struct ComponentAnimator &a = *(
          static_cast<struct ComponentAnimator *>(gsk_ecs_get(e, C_ANIMATOR)));

        gsk_AnimationSet *p_animation_set =
          (gsk_AnimationSet *)a.p_animation_set;

        // gsk_Skeleton *p_skeleton = ((gsk_Animation
        // *)a.cntAnimation)->pSkeleton;
        // TODO: FIX THIS MOTHERFUCKER
        gsk_Skeleton *p_skeleton = NULL;

        Text("Total Animations: %d", p_animation_set->animations_count);

        Text("Keyframes: %d/%d",
             a.cntKeyframeIndex,
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

// TODO: should probably be like the former
#if 0
        int item_current_idx =
          (int)p_skeleton->cnt_animation_index; // selection
#else
        int item_current_idx = (int)((gsk_Animation *)a.cntAnimation)->index;
#endif

        const char *combo_preview_value =
          p_animation_set->p_animations[item_current_idx]->name;

        if (ImGui::BeginCombo("Animations", combo_preview_value))
        {
            for (int i = 0; i < p_animation_set->animations_count; i++)
            {
                const bool is_selected = (item_current_idx == i);
                if (ImGui::Selectable(p_animation_set->p_animations[i]->name,
                                      is_selected))
                {
                    LOG_INFO("setting animation from toolbar");

                    item_current_idx  = i;
                    a.animation_index = item_current_idx;
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
    gsk::runtime::rt_set_debug_entity_id(entity.id);
    selected_entity = entity;

    visible = true;
}

void
gsk::tools::panels::ComponentViewer::draw(void)
{
    using namespace ImGui;

    gsk_Entity e = this->selected_entity;

#if 1
    {
        gsk_EntityId hovered_ent_id = gsk::runtime::rt_get_debug_entity_id();
        if (hovered_ent_id != e.id && hovered_ent_id != 0)
        {
            this->selected_entity =
              gsk_ecs_ent(this->selected_entity.ecs, hovered_ent_id);
        }
    }
#endif

    PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    Text("entity %d", e.id);
    Text("layer %d", e.ecs->p_ent_layers[e.index]);
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
