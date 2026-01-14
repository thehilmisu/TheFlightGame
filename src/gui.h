#ifndef GUI_H
#define GUI_H

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
  DebugItems dItems;

private:
  Gui();
  ~Gui();
    
};

#endif
