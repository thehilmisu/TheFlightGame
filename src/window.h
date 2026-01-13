
#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "camera.h"
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

enum KeyState {
  RELEASED,
  JUST_PRESSED,
  HELD,
};

class Window {

  Camera cam;
  double mousex, mousey; //Mouse position
  double mousedx = 0.0, mousedy = 0.0;
  glm::mat4 persp; //Global perspective matrix
  float currentFovy;
  float currentAspect;
  float currentZnear;
  float currentZfar;
  std::map<int, KeyState> keystates;
  std::map<int, KeyState> mousebuttonstates;
  double scrollspeed;

public:
  static Window& getInstance() {
    static Window instance;
    return instance;
  }

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool shouldClose() const { return glfwWindowShouldClose(mWindow); }
  void swapBuffers() { glfwSwapBuffers(mWindow); }
  void pollEvents() { glfwPollEvents(); }
  void setIsRunning(bool value) { mIsRunning = value; }
  bool isRunnning() { return mIsRunning; }
  int getWidth() const { return mWidth; }
  int getHeight() const { return mHeight; }

  double getMouseX();
  double getMouseY();
  double getMouseDX();
  double getMouseDY();
  void setMousePos(double x, double y);
  void setMouseDiff(double dx, double dy);
  Camera& getCamera();
  void updatePerspectiveMat(float fovy, float znear, float zfar, int w, int h);
  glm::mat4 getPerspective();
  float getFovy();
  float getAspect();
  float getZnear();
  float getZfar();
  void setKey(int key, KeyState keystate);
  void setButton(int button, KeyState buttonstate);
  void setScrollSpeed(double yoff);
  double getScrollSpeed();
  void updateKeyStates();
  void clearInputState();
  KeyState getKeyState(int key);
  KeyState getButtonState(int button);
  //Window callback functions
  static void handleWindowResize(GLFWwindow* window, int w, int h);
  static void cursorPosCallback(GLFWwindow* window, double x, double y);
  static void handleKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void handleMouseInput(GLFWwindow* window, int button, int action, int mods);
  static void scrollCallback(GLFWwindow* window, double xoff, double yoff);
  //Initializes mouse position
  void initMousePos();
  bool keyIsHeld(KeyState keystate);

                                   
private:
  Window();
  ~Window();

  GLFWwindow* mWindow;
  int mWidth = 800;
  int mHeight = 600;
  bool mIsRunning = true;
  std::string mTitle = "OpenGL";
  
  // Callbacks to events
  static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
  void onResize(int width, int height);
  
};
#endif 
