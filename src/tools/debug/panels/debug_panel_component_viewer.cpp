#include "debug_panel_component_viewer.hpp"

#include <entity/v1/ecs.h>
#include <imgui.h>

#include <core/device/device.h>
#include <core/drivers/alsoft/alsoft.h>
#include <core/drivers/alsoft/alsoft_debug.h>
#include <core/graphics/mesh/model.h>

void
gsk::tools::panels::ComponentViewer::show_for_entity(Entity entity)
{
    selected_entity = entity;
    visible         = true;
}

void
gsk::tools::panels::ComponentViewer::draw(void)
{
    using namespace ImGui;

    Entity e = this->selected_entity;

    PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    Text("entity %d", e.id);
    PopStyleColor();

    if (ecs_has(e, C_TRANSFORM)) {
        BeginChild("Transform", ImVec2(0, GetFontSize() * 12.0f), true);

        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Transform Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentTransform &p =
          *(static_cast<struct ComponentTransform *>(ecs_get(e, C_TRANSFORM)));
        vec3 t = GLM_VEC3_ZERO_INIT;
        DragFloat3("Position", p.position, 0.1f, -3000, 3000);
        // BeginDisabled();
        DragFloat3("Rotation", p.orientation, 0.1f, -3000, 3000);
        // EndDisabled();
        DragFloat3("Scale", p.scale, -1, 1);
        Separator();
        Text("Parent Entity");
        if (p.hasParent) {
            Text("index: %i", ((Entity *)p.parent)->index);
            Text("id: %i", ((Entity *)p.parent)->id);
        } else {
            Text("None");
        }

        EndChild();
    }
    if (ecs_has(e, C_RIGIDBODY)) {
        BeginChild("Rigidbody", ImVec2(0, GetFontSize() * 10.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Rigidbody Component");
        PopStyleColor();
        Separator();

        struct ComponentRigidbody &p =
          *(static_cast<struct ComponentRigidbody *>(ecs_get(e, C_RIGIDBODY)));

        DragFloat3("Gravity", p.gravity, 0.1f, -3000, 3000);
        DragFloat3("Velocity", p.velocity, 0.1f, -3000, 3000);
        DragFloat3("Force", p.force, 0.1f, -3000, 3000);
        DragFloat("Mass", &p.mass, 0.45f, 0.9f);

        EndChild();
    }
    if (ecs_has(e, C_MODEL)) {
        BeginChild("Model", ImVec2(0, GetFontSize() * 25.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Model Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentModel &p =
          *(static_cast<struct ComponentModel *>(ecs_get(e, C_MODEL)));

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

        Text("Meshes: %u", ((Model *)p.pModel)->meshesCount);

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

        if (CollapsingHeader("Textures")) {
            if (DEVICE_API_OPENGL) {
                int textureCount = ((Material *)p.material)->texturesCount;
                // Display textures
                for (int i = 0; i < textureCount; i++) {
                    Separator();
                    Image((void *)(intptr_t)((Material *)p.material)
                            ->textures[i]
                            ->id,
                          ImVec2(200, 200),
                          ImVec2(0, 1),
                          ImVec2(1, 0));
                    SameLine();
                    Text("File Path: %s\nDimensions: %dx%d\nType: %s",
                         ((Material *)p.material)->textures[i]->filePath,
                         ((Material *)p.material)->textures[i]->width,
                         ((Material *)p.material)->textures[i]->height,
                         "");
                }
            } // DEVICE_API_OPENGL
        }     // Textures collapsing header
        EndChild();
    }
    if (ecs_has(e, C_CAMERA)) {
        BeginChild("Camera", ImVec2(0, GetFontSize() * 10.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Camera Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentCamera &p =
          *(static_cast<struct ComponentCamera *>(ecs_get(e, C_CAMERA)));
        DragFloat("FOV", &p.fov, 0.45f, 0.9f);
        Text("Clipping");
        PushItemWidth(100);
        DragFloat("Near", &p.nearZ, 0.01, 0, 10);
        SameLine();
        DragFloat("Far", &p.farZ, 1, 0, 1000);
        EndChild();
    }
    if (ecs_has(e, C_CAMERALOOK)) {
        BeginChild("Camera Look", ImVec2(0, GetFontSize() * 6.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("CameraLook Component");
        PopStyleColor();
        Separator();
        struct ComponentCameraLook &p = *(
          static_cast<struct ComponentCameraLook *>(ecs_get(e, C_CAMERALOOK)));
        DragFloat("Sensitivity", &p.sensitivity, 0.45f, 0.9f);
        EndChild();
    }

    if (ecs_has(e, C_CAMERAMOVEMENT)) {
        BeginChild("Camera Movement", ImVec2(0, GetFontSize() * 6.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("CameraMovement Component");
        PopStyleColor();
        Separator();
        struct ComponentCameraMovement &p =
          *(static_cast<struct ComponentCameraMovement *>(
            ecs_get(e, C_CAMERAMOVEMENT)));
        DragFloat("Speed ", &p.speed, 0.45f, 0.9f);
        EndChild();
    }

    if (ecs_has(e, C_AUDIOLISTENER)) {
        BeginChild("Audio Listener");
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Audio Listener Component");
        PopStyleColor();
        Separator();

        EndChild();
    }
    if (ecs_has(e, C_AUDIOSOURCE)) {
        BeginChild("Audio Source", ImVec2(0, GetFontSize() * 10.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Audio Source Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentAudioSource &a =
          *(static_cast<struct ComponentAudioSource *>(
            ecs_get(e, C_AUDIOSOURCE)));
        // DragFloat("FOV", &a.volume, 0.45f, 0.9f);
        // DragFloat("Gain", a.gain, 0.1f, -3000, 3000);
        // DragFloat("Pitch", a.pitch, 0.1f, -3000, 3000);
        BeginDisabled();
        // InputText("Audio File Path", (char *)a.filePath, 128);
        EndDisabled();

        if (Button("Play")) { AL_CHECK(alSourcePlay(a.bufferId)); }
        SameLine();
        if (Button("Stop")) { AL_CHECK(alSourceStop(a.bufferId)); }
        if (Checkbox("Looping", (bool *)&a.looping)) {
            ;
            AL_CHECK(alSourcei(a.bufferId, AL_LOOPING, a.looping));
        }
        EndChild();
    }
    if (ecs_has(e, C_ANIMATOR)) {
        BeginChild("Animator", ImVec2(0, GetFontSize() * 10.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Animator Component");
        PopStyleColor();
        Separator();

        EndChild();
    }
    if (ecs_has(e, C_TEST)) {
        BeginChild("Lua Test Component", ImVec2(0, GetFontSize() * 8.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Lua Test Component");
        PopStyleColor();
        Separator();
        // wow, this is ridiculous..
        struct ComponentTest &p =
          *(static_cast<struct ComponentTest *>(ecs_get(e, C_TEST)));
        int movement_increment = p.movement_increment;
        float rotation_speed   = p.rotation_speed;
        Text("movement_increment: %d", movement_increment);
        Text("rotation_speed: %f", rotation_speed);

        EndChild();
    }

    if (ecs_has(e, C_WEAPON)) {
        BeginChild("Weapon Component", ImVec2(0, GetFontSize() * 8.0f), true);
        PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        Text("Weapon Component");
        PopStyleColor();
        Separator();

        struct ComponentWeapon &p =
          *(static_cast<struct ComponentWeapon *>(ecs_get(e, C_WEAPON)));

        DragFloat3("pos_starting", p.pos_starting, 0.1f, -3000, 3000);
        DragFloat3("rot_starting", p.rot_starting, 0.1f, -3000, 3000);

        EndChild();
    }
}