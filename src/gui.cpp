#include "gui.h"
#include "colors.h"
#include "game.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "window.h"
#include <glm/glm.hpp>
#include "assetloader.h"
#include "assets.h"
#include "logger.h"

Gui::Gui()
{

  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
 
  ImGui::StyleColorsDark();

  // SDL2 backend 
  Window &window = Window::getInstance();
#ifdef __ANDROID__
  const char *glsl_version = "#version 300 es";
#else
#if defined(__APPLE__)
  const char *glsl_version = "#version 150";
#else
  const char *glsl_version = "#version 130";
#endif
#endif

  ImGui_ImplSDL2_InitForOpenGL(window.getSDLWindow(), window.getGLContext());
  ImGui_ImplOpenGL3_Init(glsl_version);

  dItems.playerPosition = glm::vec3(0.0f);
  dItems.cameraPosition = glm::vec3(0.0f);
  dItems.shipCount = 0;
  dItems.balloonCount = 0;
  dItems.planeCount = 0;
  dItems.shakeIntensity = 0.0f;
  dItems.shakeDuration = 0.0f;
  dItems.isShakeButtonPressed = false;

  hudItems.fuel = 100.0f;
}

Gui::~Gui()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void Gui::newFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void Gui::render()
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::drawUI()
{
  const ImGuiViewport *viewport = ImGui::GetMainViewport();


  constexpr float sidebarwidth = 350.0f;
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(ImVec2(sidebarwidth, viewport->WorkSize.y));

  constexpr ImGuiWindowFlags sidebarflags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  if (ImGui::Begin("Debug Window", nullptr, sidebarflags))
  {
    ImGui::Text("Debug Window");
    ImGui::Separator();

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::Text("FPS : %.1f", io.Framerate);
    ImGui::Separator();
    ImGui::Text("Player Position");
    ImGui::Text("(x: %.1f, y: %.1f, z:%.1f)", dItems.playerPosition.x,
                dItems.playerPosition.y, dItems.playerPosition.z);
    ImGui::Separator();
    ImGui::Text("Camera Position");
    ImGui::Text("(x: %.1f, y: %.1f, z:%.1f)", dItems.cameraPosition.x,
                dItems.cameraPosition.y, dItems.cameraPosition.z);
    ImGui::Separator();
    ImGui::Text("Ship Count : %d", dItems.shipCount);
    ImGui::Separator();
    ImGui::Text("Balloon Count : %d", dItems.balloonCount);
    ImGui::Separator();
    ImGui::Text("Plane Count : %d", dItems.planeCount);
    ImGui::Separator();
    ImGui::SliderFloat("Shake Intensity", &dItems.shakeIntensity, 0.0f, 10.0f);
    ImGui::Separator();
    ImGui::SliderFloat("Shake Duration", &dItems.shakeDuration, 0.0f, 10.0f);
    ImGui::Separator();
    dItems.isShakeButtonPressed = ImGui::Button("Shake The Camera", ImVec2(-1.0f, 50.0f));

    ImGui::Checkbox("Rain ", &dItems.drawRain);
    ImGui::Separator();
    const char* items[] = { "Cloudy Sky", "Night Sky", "Open Sky"};
    ImGui::Combo("Skybox ", &dItems.selectedSkybox, items, IM_ARRAYSIZE(items));
    ImGui::Separator();
    if (ImGui::Button("Spawn Ship", ImVec2(-1.0f, 50.0f))) {
    }
    ImGui::Separator();
    if (ImGui::Button("Spawn Plane", ImVec2(-1.0f, 50.0f))) {
    }
    ImGui::Separator();
    if (ImGui::Button("Spawn Balloon", ImVec2(-1.0f, 50.0f))) {
    }
    ImGui::Separator();
        

    ImGui::End();
  }

  ImGui::PopStyleVar();
}

game::DeathMenuActions Gui::drawDeathMenu()
{
  game::DeathMenuActions action = game::DEATH_NONE;
  ImGui::SetNextWindowPos(ImVec2(150.0f, 500.0f), ImGuiCond_Always,
                          ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(300, 400));

  constexpr ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

  if (ImGui::Begin("GameDeathMenu", nullptr, window_flags))
  {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f); // Round the buttons

    // Title Text
    ImGui::SetWindowFontScale(2.0f);
    const float text_width = ImGui::CalcTextSize("River Raid 3D").x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);
    ImGui::Text("River Raid 3D");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("TRY AGAIN", ImVec2(-1.0f, 50.0f))) {
      action = game::TRY_AGAIN;
    }

    if (ImGui::Button("LOAD GAME", ImVec2(-1.0f, 50.0f))) {
      // Load menu logic
    }

    if (ImGui::Button("EXIT TO MAIN MENU", ImVec2(-1.0f, 50.0f))) {
      action = game::DEATH_EXIT_TO_MAINMENU;
    }

    ImGui::Spacing();

    if (ImGui::Button("EXIT", ImVec2(-1.0f, 50.0f))) {
      action = game::DEATH_EXIT;
    }

    ImGui::PopStyleVar();
    ImGui::End();
  }
  return action;
}

game::PauseMenuActions Gui::drawPauseMenu()
{
  game::PauseMenuActions action = game::NONE;

  ImGui::SetNextWindowPos(ImVec2(150.0f, 500.0f), ImGuiCond_Always,
                          ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(300, 400));

  // Use flags to remove the typical window frame for a cleaner "Game Menu" look
  constexpr ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

  if (ImGui::Begin("GamePauseMenu", nullptr, window_flags)) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f); // Round the buttons

    // Title Text
    ImGui::SetWindowFontScale(2.0f);
    float text_width = ImGui::CalcTextSize("River Raid 3D").x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);
    ImGui::Text("River Raid 3D");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("RESUME GAME", ImVec2(-1.0f, 50.0f))) {
      action = game::RESUME;
    }

    if (ImGui::Button("LOAD GAME", ImVec2(-1.0f, 50.0f))) {
      // Load menu logic
    }

    if (ImGui::Button("EXIT TO MAIN MENU", ImVec2(-1.0f, 50.0f))) {
      action = game::EXIT_TO_MAINMENU;
    }

    ImGui::Spacing();

    if (ImGui::Button("EXIT", ImVec2(-1.0f, 50.0f))) {
      action = game::EXIT;
    }

    ImGui::PopStyleVar();
    ImGui::End();
  }

  return action;
}

game::MainMenuActions Gui::drawMainMenu()
{
  game::MainMenuActions action = game::NONE_SELECTED;
  const Window &window = Window::getInstance();
  int w = window.getWidth();
  int h = window.getHeight();

  float pos_x = (w * 0.3f) + 100.0f;
  const auto pos_y = h + 75.0f;

  ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), ImGuiCond_Always,
                          ImVec2(1.0f, 1.0f));
  ImGui::SetNextWindowSize(ImVec2((w * 0.30f), (h * 0.7f)));

  // Use flags to remove the typical window frame for a cleaner "Game Menu" look
  constexpr ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

  if (ImGui::Begin("GameMainMenu", nullptr, window_flags)) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    // ImGui::PushFont(FONTS->getFontData("jetbrainsmono_bold"));

    // Title Text
    ImGui::SetWindowFontScale(2.5f);
    const float text_width = ImGui::CalcTextSize("River Raid 3D").x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);
    ImGui::Text("River Raid 3D");
    ImGui::SetWindowFontScale(2.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("NEW GAME", ImVec2(-1.0f, 50.0f))) {
      action = game::START_GAME;
    }

    if (ImGui::Button("LOAD GAME", ImVec2(-1.0f, 50.0f))) {
      action = game::LOAD_GAME;
    }

    if (ImGui::Button("HANGAR", ImVec2(-1.0f, 50.0f))) {
      action = game::HANGAR;
    }

    if (ImGui::Button("OPTIONS", ImVec2(-1.0f, 50.0f))) {
      action = game::OPTIONS;
    }

    if (ImGui::Button("EXIT", ImVec2(-1.0f, 50.0f))) {
      action = game::EXIT_GAME;
    }

    ImGui::Spacing();
    // ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::End();
  }

  
  return action;
}


game::MainMenuActions Gui::drawPlaneSelectionUI(const gameobjects::Player::PlayerModel& pModel) {

  game::MainMenuActions action = game::NONE_SELECTED;
  const Window &window = Window::getInstance();
  const int w = window.getWidth();
  const int h = window.getHeight();

  float pos_x = (w / 2.0f);
  auto pos_y = h - 50.0f;
  ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), ImGuiCond_Always,
                         ImVec2(0.5f, 1.0f));
  ImGui::SetNextWindowSize(ImVec2(900, 75));

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

  if (ImGui::Begin("PlaneArrows", nullptr, window_flags)) {
   ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
   ImGui::SetWindowFontScale(2.0f);

    if (ImGui::Button("<<", ImVec2(150.0f, 50.0f))) {
      action = game::CHANGE_PLANE_MINUS;
    }

    ImGui::SameLine();
    
    if (ImGui::Button(">>", ImVec2(150.0f, 50.0f))) {
      action = game::CHANGE_PLANE_PLUS;
    }

    ImGui::SameLine(0.0f, 250.0f);
    if (ImGui::Button("Select Plane", ImVec2(250.0, 50.0f))) {
      action = game::PLANE_SELECTED;  
    }
    
    ImGui::PopStyleVar();
    ImGui::End();
    
    // Selected Plane information
    pos_x = (w / 2.0f) + 500.0f;
    pos_y = 300.0f;
    
    ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), ImGuiCond_Always,
                         ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(500, 250));

    window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove;

     if (ImGui::Begin("PlaneInformation", nullptr, window_flags)) {
       ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
       ImGui::SetWindowFontScale(2.5f);

       //TODO: fix this potential issue
       ImGui::Text(pModel.plane_name.c_str());
       ImGui::Separator();

       ImGui::SetWindowFontScale(1.5f);
       //TODO: fix this potential issue
       ImGui::Text(pModel.description.c_str());
       
     }
     ImGui::PopStyleVar();
     ImGui::End();
 }

 return action;
}

void Gui::drawHUD()
{

  ImGuiViewport *viewport = ImGui::GetMainViewport();


  constexpr ImGuiWindowFlags hudWindowFlags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoInputs |
                                    // ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoSavedSettings;

  // TOP-CENTER: Elapsed Time
  {
    constexpr float padding = 10.0f;
    const float textWidth = ImGui::CalcTextSize("00:00").x;

    ImGui::SetNextWindowPos(ImVec2(viewport->GetCenter().x - textWidth / 2.0f,
                                   viewport->WorkPos.y + padding));

    if (ImGui::Begin("HUD_TopCenter", nullptr, hudWindowFlags)) {
      ImGui::SetWindowFontScale(1.6f);

      const double time = hudItems.elapsedTime;
      const int totalSeconds = static_cast<int>(time);

      const int minutes = totalSeconds / 60;
      const int seconds = totalSeconds % 60;
      ImGui::Text("%02d:%02d", minutes, seconds);
      ImGui::SameLine();

      ImGui::End();
    }
  }

  // BOTTOM LEFT: Altitude
  {
    float textWidth = ImGui::CalcTextSize("Altitude : 9999").x;
    ImGui::SetNextWindowPos(ImVec2(textWidth / 2.0f,
                                   viewport->Size.y - 300.0f));

    if (ImGui::Begin("HUD_BottomLeft", nullptr, hudWindowFlags)) {
      ImGui::SetWindowFontScale(1.2f);
      ImGui::Text("Altitude : %.2f", hudItems.altitude);
      ImGui::SameLine();

      ImGui::End();
    }
  }
}
