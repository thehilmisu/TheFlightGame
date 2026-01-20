#ifndef GUI_H
#define GUI_H

#include "game.h"
#include "glm/ext/vector_float3.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

struct DebugItems{
  glm::vec3 playerPosition;
  glm::vec3 cameraPosition;
  int bulletCount;
};

struct HUDItems {
  unsigned int health;
  float fuel;
  float speed;
  unsigned int bulletCount;
  float altitude;
  unsigned int score;
  bool crashed;
};

class Gui {

public:
  static Gui& getInstance() {
    static Gui instance;
    return instance;
  }

  Gui(const Gui&) = delete;
  Gui& operator=(const Gui&) = delete;

  void newFrame();
  void render();

  void drawUI();
  void drawHUD();
  game::GameMode drawMainMenu();
  DebugItems dItems;
  HUDItems hudItems;

private:
  Gui();
  ~Gui();
    
};

#endif
