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
#include "settings.h"
#include "audio.h"

// Typography.
// The HUD readouts use the monospaced face so that digits keep a fixed width -
// a proportional font makes the clock and score jitter sideways every time a
// digit changes. Menus use Armata, which is the game's display face.
// ImGui 1.92 rasterises fonts on demand, so these sizes are exact pixel sizes
// rather than a scale factor applied to a bitmap font, which is what made the
// old text look soft.
static const char *HUD_FONT = "jetbrainsmono_bold";
static const char *MENU_FONT = "armata_medium";
static const char *MENU_FONT_SMALL = "armata_small";

constexpr float HUD_LARGE = 30.0f;
constexpr float HUD_MEDIUM = 21.0f;
constexpr float HUD_SMALL = 17.0f;
constexpr float MENU_TITLE = 40.0f;
constexpr float MENU_BUTTON = 26.0f;
constexpr float MENU_LABEL = 19.0f;

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
    FONTS->pushFont(MENU_FONT, MENU_TITLE);
    const float text_width = ImGui::CalcTextSize("River Raid 3D").x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);
    ImGui::TextUnformatted("River Raid 3D");
    FONTS->popFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Result of the run that just ended
    FONTS->pushFont(HUD_FONT, HUD_MEDIUM);
    ImGui::Text("SCORE %u", hudItems.score);
    ImGui::Text("BEST  %u", hudItems.highScore);
    if (hudItems.newHighScore)
      ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "NEW BEST!");
    FONTS->popFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    FONTS->pushFont(MENU_FONT, MENU_BUTTON);

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

    FONTS->popFont();
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
    FONTS->pushFont(MENU_FONT, MENU_TITLE);
    float text_width = ImGui::CalcTextSize("River Raid 3D").x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);
    ImGui::TextUnformatted("River Raid 3D");
    FONTS->popFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    FONTS->pushFont(MENU_FONT, MENU_BUTTON);

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

    FONTS->popFont();
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

    // Title Text
    FONTS->pushFont(MENU_FONT, MENU_TITLE);
    const float text_width = ImGui::CalcTextSize("River Raid 3D").x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);
    ImGui::TextUnformatted("River Raid 3D");
    FONTS->popFont();

    // Something to beat, shown only once the player has actually set a score
    if (SETTINGS.getHighScore() > 0) {
      FONTS->pushFont(HUD_FONT, MENU_LABEL);
      ImGui::Text("BEST %u", SETTINGS.getHighScore());
      FONTS->popFont();
    }

    FONTS->pushFont(MENU_FONT, MENU_BUTTON);

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
    FONTS->popFont();
    ImGui::PopStyleVar();
    ImGui::End();
  }


  return action;
}

game::MainMenuActions Gui::drawOptionsMenu() {
  game::MainMenuActions action = game::NONE_SELECTED;
  const Window &window = Window::getInstance();
  int w = window.getWidth();
  int h = window.getHeight();

  float pos_x = (w * 0.5f) + 100.0f;
  const auto pos_y = h + 25.0f;

  ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), ImGuiCond_Always,
                          ImVec2(1.0f, 1.0f));
  ImGui::SetNextWindowSize(ImVec2((w * 0.50f), (h * 0.7f)));

  // Use flags to remove the typical window frame for a cleaner "Game Menu" look
  constexpr ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

  if (ImGui::Begin("GameOptionsMenu", nullptr, window_flags)) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

    // Title Text
    FONTS->pushFont(MENU_FONT, MENU_TITLE);
    const float text_width = ImGui::CalcTextSize("OPTIONS").x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);
    ImGui::TextUnformatted("OPTIONS");
    FONTS->popFont();

    FONTS->pushFont(MENU_FONT, MENU_BUTTON);

    // Options content. These are bound to the saved settings and applied to
    // the audio system as soon as they change, so the controls are audible
    // immediately rather than only after a restart.
    bool soundEnabled = SETTINGS.soundIsEnabled();
    int volume = SETTINGS.getVolume();

    if (ImGui::Checkbox("Sound", &soundEnabled)) {
      SETTINGS.setSoundEnabled(soundEnabled);
      SFX->applySettings();
    }
    if (soundEnabled) {
      ImGui::SameLine();
      if (ImGui::SliderInt("Volume", &volume, 0, 100)) {
        SETTINGS.setVolume(volume);
        SFX->applySettings();
      }
      // Give the player something to judge the new volume against
      if (ImGui::IsItemDeactivatedAfterEdit())
        SFX->playid("click");
    }
    ImGui::Separator();
    if (ImGui::Button("Credits", ImVec2(-1.0f, 50.0f))) {
      // Not wired up yet - see the credits/licensing note. It deliberately no
      // longer returns BACK_TO_MAINMENU, which used to throw the player out of
      // the options screen when they clicked it.
    }
    ImGui::Separator();
    if (ImGui::Button("BACK TO MAIN MENU", ImVec2(-1.0f, 50.0f))) {
      SETTINGS.save();
      action = game::BACK_TO_MAINMENU;
    }


    ImGui::Spacing();
    FONTS->popFont();
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
   FONTS->pushFont(MENU_FONT, MENU_BUTTON);

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

    FONTS->popFont();
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

       //TextUnformatted, not Text: these strings are data, and passing them as
       //a format string would interpret any '%' inside them
       FONTS->pushFont(MENU_FONT, MENU_BUTTON);
       ImGui::TextUnformatted(pModel.plane_name.c_str());
       FONTS->popFont();
       ImGui::Separator();

       FONTS->pushFont(MENU_FONT_SMALL, MENU_LABEL);
       ImGui::TextWrapped("%s", pModel.description.c_str());
       FONTS->popFont();
     }
     ImGui::PopStyleVar();
     ImGui::End();
 }

 return action;
}

void Gui::drawHUD()
{
  const Window &window = Window::getInstance();
  const int w = window.getWidth();
  const int h = window.getHeight();

  // AlwaysAutoResize makes each readout shrink-wrap its text, so the panel
  // grows with the score instead of being a fixed image that the content has
  // to be made to fit.
  constexpr ImGuiWindowFlags hudWindowFlags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoInputs |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_AlwaysAutoResize;

  // A dark translucent panel drawn by ImGui itself: it follows the text
  // exactly, stays readable over bright sky and dark terrain alike, and needs
  // no artwork.
  auto pushHudPanelStyle = []() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.05f, 0.07f, 0.55f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.75f, 0.82f, 0.90f, 0.18f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 3.0f));
  };
  auto popHudPanelStyle = []() {
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(2);
  };

  auto centeredText = [](const char *text) {
    const float textWidth = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textWidth) * 0.5f);
    ImGui::TextUnformatted(text);
  };

  char line[64];

  // TOP CENTRE: run time, score and best.
  // The pivot keeps it centred no matter how wide the score grows.
  {
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(w) * 0.5f, 12.0f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    pushHudPanelStyle();
    if (ImGui::Begin("HUD_TopCenter", nullptr, hudWindowFlags)) {
      const int totalSeconds = static_cast<int>(hudItems.elapsedTime);
      snprintf(line, sizeof(line), "%02d:%02d", totalSeconds / 60, totalSeconds % 60);
      FONTS->pushFont(HUD_FONT, HUD_LARGE);
      centeredText(line);
      FONTS->popFont();

      FONTS->pushFont(HUD_FONT, HUD_SMALL);
      snprintf(line, sizeof(line), "SCORE %u", hudItems.score);
      // Gold while this run is beating the record, so the readout reacts to
      // what it is showing
      const bool beatingBest = hudItems.score > hudItems.highScore;
      const float textWidth = ImGui::CalcTextSize(line).x;
      ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textWidth) * 0.5f);
      if (beatingBest)
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.25f, 1.0f), "%s", line);
      else
        ImGui::TextUnformatted(line);

      snprintf(line, sizeof(line), "BEST  %u", hudItems.highScore);
      centeredText(line);
      FONTS->popFont();

      ImGui::End();
    }
    popHudPanelStyle();
  }

  // BOTTOM LEFT: altitude, anchored above the attitude indicator
  {
    ImGui::SetNextWindowPos(ImVec2(16.0f, static_cast<float>(h) - 228.0f),
                            ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    pushHudPanelStyle();
    if (ImGui::Begin("HUD_BottomLeft", nullptr, hudWindowFlags)) {
      FONTS->pushFont(HUD_FONT, HUD_MEDIUM);
      snprintf(line, sizeof(line), "ALT %.0f", hudItems.altitude);
      ImGui::TextUnformatted(line);
      FONTS->popFont();
      ImGui::End();
    }
    popHudPanelStyle();
  }
}
