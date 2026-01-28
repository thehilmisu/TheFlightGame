#include "window.h"
#include <SDL.h>
#include <iostream>
#include "imgui_impl_sdl2.h"

Window::Window(){

  // SDL2 initialization
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    std::cerr << "Failed to initialize SDL2: " << SDL_GetError() << std::endl;
    return;
  }

  // Set OpenGL attributes
#ifdef __ANDROID__
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // Create window
  Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
#ifdef __ANDROID__
  windowFlags |= SDL_WINDOW_FULLSCREEN;
#endif

  mWindow = SDL_CreateWindow(
    mTitle.c_str(),
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    mWidth,
    mHeight,
    windowFlags
  );

  if (!mWindow) {
    std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return;
  }

  // Create OpenGL context
  mGLContext = SDL_GL_CreateContext(mWindow);
  if (!mGLContext) {
    std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
    return;
  }

  // Enable vsync
  SDL_GL_SetSwapInterval(1);

  mShouldClose = false;

  cam = Camera(glm::vec3(0.0f, 360.0f, 0.0f));
  mousex = 0.0;
  mousey = 0.0;
  persp = glm::mat4(1.0f);

  currentFovy = 0.0f;
  currentAspect = 0.0f;
  currentZnear = 0.0f;
  currentZfar = 0.0f;

  scrollspeed = 0.0f;
 }

Window::~Window() {
  if(mGLContext) {
    SDL_GL_DeleteContext(mGLContext);
  }
  if(mWindow) {
    SDL_DestroyWindow(mWindow);
  }
  SDL_Quit();
}

void Window::onResize(int width, int height){
  mWidth = width;
  mHeight = height;

  glViewport(0, 0, width, height);

  updatePerspectiveMat(currentFovy, currentZnear, currentZfar, width, height);
}

void Window::updatePerspectiveMat(float fovy, float znear, float zfar, int w, int h)
{
	const float aspect = float(w) / float(h);
	persp = glm::perspective(fovy, aspect, znear, zfar);

	currentFovy = fovy;
	currentAspect = aspect;
	currentZnear = znear;
	currentZfar = zfar;
}

// Instance implementations
double Window::getMouseXImpl()
{
    return mousex;
}

double Window::getMouseYImpl()
{
  return mousey;
}

double Window::getMouseDXImpl()
{
  return mousedx;
}

double Window::getMouseDYImpl()
{
  return mousedy;
}

// Static wrappers
double Window::getMouseX()
{
    return getInstance().getMouseXImpl();
}

double Window::getMouseY()
{
  return getInstance().getMouseYImpl();
}

double Window::getMouseDX()
{
  return getInstance().getMouseDXImpl();
}

double Window::getMouseDY()
{
  return getInstance().getMouseDYImpl();
}

double Window::getScrollSpeed()
{
  return scrollspeed;
}

// Instance implementations
void Window::setMousePosImpl(double x, double y)
{
  mousex = x;
  mousey = y;
}

void Window::setMouseDiffImpl(double dx, double dy)
{
  mousedx = dx;
  mousedy = dy;
}

// Static wrappers
void Window::setMousePos(double x, double y)
{
  getInstance().setMousePosImpl(x, y);
}

void Window::setMouseDiff(double dx, double dy)
{
  getInstance().setMouseDiffImpl(dx, dy);
}

Camera& Window::getCamera()
{
  return cam;
}

float Window::getFovy()
{
	return currentFovy;
}

float Window::getAspect()
{
	return currentAspect;
}

float Window::getZnear()
{
	return currentZnear;
}

float Window::getZfar()
{
	return currentZfar;
}

void Window::setKeyImpl(int key, KeyState keystate)
{
	keystates[key] = keystate;
}

void Window::setButtonImpl(int button, KeyState buttonstate)
{
	mousebuttonstates[button] = buttonstate;
}

void Window::setScrollSpeedImpl(double yoff)
{
	scrollspeed = yoff;
}

// Static wrappers
void Window::setKey(int key, KeyState keystate)
{
	getInstance().setKeyImpl(key, keystate);
}

void Window::setButton(int button, KeyState buttonstate)
{
	getInstance().setButtonImpl(button, buttonstate);
}

void Window::setScrollSpeed(double yoff)
{
	getInstance().setScrollSpeedImpl(yoff);
}

void Window::updateKeyStates()
{
	for(auto &keystate : keystates)
		if(keystate.second == JUST_PRESSED)
			keystate.second = HELD;

	for(auto &buttonstate : mousebuttonstates)
		if(buttonstate.second == JUST_PRESSED)
			buttonstate.second = HELD;

	scrollspeed = 0.0;

	mousedx = 0.0;
	mousedy = 0.0;
}

void Window::clearInputState()
{
	keystates = {};
	mousebuttonstates = {};
}

KeyState Window::getKeyState(int key)
{
	return keystates[key];
}

KeyState Window::getButtonState(int button)
{
	return mousebuttonstates[button];
}

glm::mat4 Window::getPerspective()
{
	return persp;
}

bool Window::keyIsHeld(KeyState keystate)
{
	return keystate == JUST_PRESSED || keystate == HELD;
}

void Window::setCursorInputMode(int mode){
  if(mode == CURSOR_DISABLED) {
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
  } else if(mode == CURSOR_HIDDEN) {
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(SDL_DISABLE);
  } else { // CURSOR_NORMAL
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(SDL_ENABLE);
  }
}

// SDL2 method implementations
bool Window::shouldClose() const {
  return mShouldClose;
}

void Window::swapBuffers() {
  SDL_GL_SwapWindow(mWindow);
}

void Window::pollEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    // Let ImGui process the event first
    ImGui_ImplSDL2_ProcessEvent(&event);

    // Then process window events
    processSDLEvent(event);
  }
}

void Window::processSDLEvent(SDL_Event& event) {
  switch (event.type) {
    case SDL_QUIT:
      mShouldClose = true;
      break;

    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
        onResize(event.window.data1, event.window.data2);
      }
      break;

    case SDL_KEYDOWN:
      if (!event.key.repeat) {
        setKey(event.key.keysym.sym, JUST_PRESSED);
      }
      break;

    case SDL_KEYUP:
      setKey(event.key.keysym.sym, RELEASED);
      break;

    case SDL_MOUSEMOTION:
      {
        double dx = event.motion.xrel;
        double dy = event.motion.yrel;
        setMousePos(event.motion.x, event.motion.y);
        setMouseDiff(dx, dy);
      }
      break;

    case SDL_MOUSEBUTTONDOWN:
      setButton(event.button.button, JUST_PRESSED);
      break;

    case SDL_MOUSEBUTTONUP:
      setButton(event.button.button, RELEASED);
      break;

    case SDL_MOUSEWHEEL:
      setScrollSpeed(event.wheel.y);
      break;

#ifdef __ANDROID__
    case SDL_FINGERDOWN:
      // Convert touch to mouse for Android
      {
        int w, h;
        SDL_GetWindowSize(mWindow, &w, &h);
        double x = event.tfinger.x * w;
        double y = event.tfinger.y * h;
        setMousePos(x, y);
        setButton(SDL_BUTTON_LEFT, JUST_PRESSED);
      }
      break;

    case SDL_FINGERUP:
      setButton(SDL_BUTTON_LEFT, RELEASED);
      break;

    case SDL_FINGERMOTION:
      {
        int w, h;
        SDL_GetWindowSize(mWindow, &w, &h);
        double x = event.tfinger.x * w;
        double y = event.tfinger.y * h;
        double dx = event.tfinger.dx * w;
        double dy = event.tfinger.dy * h;
        setMousePos(x, y);
        setMouseDiff(dx, dy);
      }
      break;
#endif

    default:
      break;
  }
}

void Window::initMousePos()
{
  int mousex, mousey;
  SDL_GetMouseState(&mousex, &mousey);
  setMousePos(mousex, mousey);
}
