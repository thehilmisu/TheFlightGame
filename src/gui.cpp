#include "gui.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include "colors.h"

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

  hudItems.fuel = 100.0f;
  
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
      // ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoSavedSettings;

  // TOP-CENTER: FUEL
  {
    ImGui::SetNextWindowPos(ImVec2(viewport->GetCenter().x - 150.0f,
                                   viewport->WorkPos.y + padding));

    if (ImGui::Begin("HUD_TopCenter", nullptr, hudFlags)) {
      ImGui::SetWindowFontScale(1.2f);

      // fuel bar with color coding
      float fuelPercent = hudItems.fuel / 100.0f;
      ImVec4 fuelBarColor;
      if (fuelPercent > 0.6f)
        fuelBarColor = COLOR_GREEN; // Green
      else if (fuelPercent > 0.3f)
        fuelBarColor = COLOR_YELLOW; // Yellow
      else
        fuelBarColor = COLOR_RED; // Red

      ImGui::Text("Fuel : ");
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_PlotHistogram, fuelBarColor);
      ImGui::ProgressBar(fuelPercent, ImVec2(300, 20));
      ImGui::PopStyleColor();

      ImGui::End();
    }
  }
 
  // TOP-RIGHT: Score + Ammo
  {
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x - 300,
                                   viewport->WorkPos.y + padding));

    if (ImGui::Begin("HUD_TopRight", nullptr, hudFlags)) {
      ImGui::SetWindowFontScale(1.2f);

      ImGui::Text("Score: %u", hudItems.score);

      ImGui::End();
    }
  }

  // BOTTOM-RIGHT: Altitude
  {
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x - 300.0f + padding,
                                   viewport->WorkPos.y + viewport->WorkSize.y - 120));

    if (ImGui::Begin("HUD_BottomLeft", nullptr, hudFlags)) {
      ImGui::SetWindowFontScale(1.2f);

      ImGui::Text("Altitude: %.1f", hudItems.altitude);
      ImGui::Text("Speed: %.1f", hudItems.speed);

      // health bar with color coding
      float healthPercent = hudItems.health / 100.0f;
      ImVec4 healthBarColor;
      if (healthPercent > 0.6f)
        healthBarColor = COLOR_GREEN; // Green
      else if (healthPercent > 0.3f)
        healthBarColor = COLOR_YELLOW; // Yellow
      else
        healthBarColor = COLOR_RED; // Red
        
      ImGui::Text("Health : ");
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_PlotHistogram, healthBarColor);
      ImGui::ProgressBar(healthPercent, ImVec2(150, 20));
      ImGui::PopStyleColor();
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
