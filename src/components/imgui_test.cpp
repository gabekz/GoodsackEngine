/*
#include "imgui_test.h"

#include <core/ecs.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static void init(Entity e) {
// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(e.ecs->renderer->window, true);
    ImGui_ImplOpenGL3_Init("440");
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();


}

static void render(ECS *ecs) {

}


void s_imgui_test_init(ECS *ecs) {
    ecs_component_register(ecs, C_IMGUI_TEST, sizeof(struct ComponentImgui));
    ecs_system_register(ecs, ((ECSSystem){
        .init       = (ECSSubscriber) init,
        .destroy    = NULL,
        .render     = (ECSSubscriber) render,
        .update     = NULL,
    }));
}
*/
