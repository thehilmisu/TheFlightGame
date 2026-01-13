#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

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

private:
  Gui();
  ~Gui();
    
};

#endif
