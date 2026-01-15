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

  dItems.playerPosition = glm::vec3(0.0f);
  dItems.cameraPosition = glm::vec3(0.0f);
  dItems.bulletCount = 0;
  
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
    ImGui::Separator();
    ImGui::Text("Player Position");
    ImGui::Text("(x: %.1f, y: %.1f, z:%.1f)",
                dItems.playerPosition.x,
                dItems.playerPosition.y,
                dItems.playerPosition.z);
    ImGui::Separator();
    ImGui::Text("Camera Position");
    ImGui::Text("(x: %.1f, y: %.1f, z:%.1f)",
                dItems.cameraPosition.x,
                dItems.cameraPosition.y,
                dItems.cameraPosition.z);
    ImGui::Separator();
    ImGui::Text("Bullet Count : %d", dItems.bulletCount);
    
    

    ImGui::End();
  }

  ImGui::PopStyleVar();
}

void Gui::drawHUD(){
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  const float padding = 10.0f;

  ImGuiWindowFlags hudFlags =
      ImGuiWindowFlags_NoDecoration |
      ImGuiWindowFlags_NoInputs |
      ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoSavedSettings;

  // TOP-LEFT: Health + Speed
  {
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + padding,
                                   viewport->WorkPos.y + padding));

    if (ImGui::Begin("HUD_TopLeft", nullptr, hudFlags)) {
      ImGui::SetWindowFontScale(1.2f);

      // Health bar with color coding
      float healthPercent = hudItems.health / float(hudItems.maxHealth);
      ImVec4 healthColor;
      if (healthPercent > 0.6f)
        healthColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
      else if (healthPercent > 0.3f)
        healthColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
      else
        healthColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red

      ImGui::Text("HP: %u/%u", hudItems.health, hudItems.maxHealth);
      ImGui::PushStyleColor(ImGuiCol_PlotHistogram, healthColor);
      ImGui::ProgressBar(healthPercent, ImVec2(200, 20));
      ImGui::PopStyleColor();

      // Speed
      ImGui::Text("Speed: %.1f", hudItems.speed);

      ImGui::End();
    }
  }

  // TOP-RIGHT: Score + Ammo
  {
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 220,
                                   viewport->WorkPos.y + padding));

    if (ImGui::Begin("HUD_TopRight", nullptr, hudFlags)) {
      ImGui::SetWindowFontScale(1.2f);

      ImGui::Text("Score: %u", hudItems.score);
      ImGui::Text("Ammo: %u", hudItems.bulletCount);

      ImGui::End();
    }
  }

  // BOTTOM-LEFT: Altitude
  {
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + padding,
                                   viewport->WorkPos.y + viewport->WorkSize.y - 60));

    if (ImGui::Begin("HUD_BottomLeft", nullptr, hudFlags)) {
      ImGui::SetWindowFontScale(1.2f);

      ImGui::Text("Altitude: %.1f", hudItems.altitude);

      ImGui::End();
    }
  }

  // Optional: Crash indicator
  if (hudItems.crashed) {
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x / 2 - 100,
                                   viewport->WorkPos.y + viewport->WorkSize.y / 2 - 50));

    ImGuiWindowFlags crashFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("HUD_Crash", nullptr, crashFlags)) {
      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.5f, 0.0f, 0.0f, 0.7f));
      ImGui::SetWindowFontScale(2.0f);
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CRASHED!");
      ImGui::PopStyleColor();
      ImGui::End();
    }
  }
}
