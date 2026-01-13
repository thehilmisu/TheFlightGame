#include "gui.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>

Gui::Gui() {

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  #if defined(__APPLE__)
    const char* glsl_version = "#version 150";
  #else
    const char* glsl_version = "#version 130";
  #endif

  ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
  ImGui_ImplOpenGL3_Init(glsl_version);
  
}

Gui::~Gui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Gui::newFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Gui::render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::drawUI() {
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  float sidebarwidth = 250.0f;
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(ImVec2(sidebarwidth, viewport->WorkSize.y));

  ImGuiWindowFlags sidebarflags =
      ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoMove     |
      ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  if (ImGui::Begin("Debug Window", nullptr, sidebarflags)) {
    ImGui::Text("Debug Window");
    ImGui::Separator();
    
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    ImGui::Text("FPS : %.1f", io.Framerate);
    

    ImGui::End();
  }

  ImGui::PopStyleVar();
}
