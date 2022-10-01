#include "navbar.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void navbar_render(Renderer *renderer) {
    if(ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Exit")) {
               glfwSetWindowShouldClose(renderer->window, GLFW_TRUE);
            }
            ImGui::EndMenu();
          }

         if (ImGui::BeginMenu("Scene")) {
            if(ImGui::MenuItem("Change Scene")) {
            }
            if(ImGui::MenuItem("Lighting")) {
            }
            if(ImGui::MenuItem("Entities")) {
            }
            ImGui::EndMenu();
        }

         if (ImGui::BeginMenu("Debug")) {
            ImGui::EndMenu();
        }
       
        ImGui::EndMainMenuBar();
    }
}
