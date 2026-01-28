#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#ifdef __ANDROID__
#include <GLES3/gl3.h>
#else
#include <SDL_opengl.h>
#endif

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

  bool shouldClose() const;
  void swapBuffers();
  void pollEvents();
  void setIsRunning(bool value) { mIsRunning = value; }
  bool isRunnning() { return mIsRunning; }
  int getWidth() const { return mWidth; }
  int getHeight() const { return mHeight; }

  static double getMouseX();
  static double getMouseY();
  static double getMouseDX();
  static double getMouseDY();
  static void setMousePos(double x, double y);
  static void setMouseDiff(double dx, double dy);
  Camera& getCamera();
  void updatePerspectiveMat(float fovy, float znear, float zfar, int w, int h);
  glm::mat4 getPerspective();
  float getFovy();
  float getAspect();
  float getZnear();
  float getZfar();
  static void setKey(int key, KeyState keystate);
  static void setButton(int button, KeyState buttonstate);
  static void setScrollSpeed(double yoff);
  double getScrollSpeed();
  void updateKeyStates();
  void clearInputState();
  KeyState getKeyState(int key);
  KeyState getButtonState(int button);
  void initMousePos();
  bool keyIsHeld(KeyState keystate);
  void setCursorInputMode(int mode);

  SDL_Window* getSDLWindow() { return mWindow; }
  SDL_GLContext getGLContext() { return mGLContext; }

private:
  Window();
  ~Window();

  SDL_Window* mWindow;
  SDL_GLContext mGLContext;
  bool mShouldClose = false;

  int mWidth = 1280;
  int mHeight = 720;
  bool mIsRunning = true;
  std::string mTitle = "River Raid 3D";

  void onResize(int width, int height);

  // Private instance method implementations for static wrappers
  double getMouseXImpl();
  double getMouseYImpl();
  double getMouseDXImpl();
  double getMouseDYImpl();
  void setMousePosImpl(double x, double y);
  void setMouseDiffImpl(double dx, double dy);
  void setKeyImpl(int key, KeyState keystate);
  void setButtonImpl(int button, KeyState buttonstate);
  void setScrollSpeedImpl(double yoff);

  // SDL2 specific methods
  void processSDLEvent(SDL_Event& event);
  int mapSDL2KeyToGLFW(int sdlKey);
  int mapSDL2MouseButtonToGLFW(int sdlButton);
};
#endif
