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
  // Map GLFW cursor modes to SDL2
  // GLFW_CURSOR_DISABLED = 0x00034003
  // GLFW_CURSOR_NORMAL = 0x00034001
  if(mode == 0x00034003) { // GLFW_CURSOR_DISABLED
    SDL_SetRelativeMouseMode(SDL_TRUE);
  } else {
    SDL_SetRelativeMouseMode(SDL_FALSE);
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
        setKey(mapSDL2KeyToGLFW(event.key.keysym.sym), JUST_PRESSED);
      }
      break;

    case SDL_KEYUP:
      setKey(mapSDL2KeyToGLFW(event.key.keysym.sym), RELEASED);
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
      setButton(mapSDL2MouseButtonToGLFW(event.button.button), JUST_PRESSED);
      break;

    case SDL_MOUSEBUTTONUP:
      setButton(mapSDL2MouseButtonToGLFW(event.button.button), RELEASED);
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
        setButton(0, JUST_PRESSED); // Left mouse button
      }
      break;

    case SDL_FINGERUP:
      setButton(0, RELEASED);
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

int Window::mapSDL2KeyToGLFW(int sdlKey) {
  // Map common SDL2 keys to GLFW key codes
  // GLFW key codes are defined in keycodes.h
  switch (sdlKey) {
    case SDLK_SPACE: return 32;  // GLFW_KEY_SPACE
    case SDLK_ESCAPE: return 256; // GLFW_KEY_ESCAPE
    case SDLK_RETURN: return 257; // GLFW_KEY_ENTER
    case SDLK_TAB: return 258; // GLFW_KEY_TAB
    case SDLK_BACKSPACE: return 259; // GLFW_KEY_BACKSPACE
    case SDLK_INSERT: return 260; // GLFW_KEY_INSERT
    case SDLK_DELETE: return 261; // GLFW_KEY_DELETE
    case SDLK_RIGHT: return 262; // GLFW_KEY_RIGHT
    case SDLK_LEFT: return 263; // GLFW_KEY_LEFT
    case SDLK_DOWN: return 264; // GLFW_KEY_DOWN
    case SDLK_UP: return 265; // GLFW_KEY_UP
    case SDLK_PAGEUP: return 266; // GLFW_KEY_PAGE_UP
    case SDLK_PAGEDOWN: return 267; // GLFW_KEY_PAGE_DOWN
    case SDLK_HOME: return 268; // GLFW_KEY_HOME
    case SDLK_END: return 269; // GLFW_KEY_END
    case SDLK_LSHIFT: return 340; // GLFW_KEY_LEFT_SHIFT
    case SDLK_LCTRL: return 341; // GLFW_KEY_LEFT_CONTROL
    case SDLK_LALT: return 342; // GLFW_KEY_LEFT_ALT
    case SDLK_RSHIFT: return 344; // GLFW_KEY_RIGHT_SHIFT
    case SDLK_RCTRL: return 345; // GLFW_KEY_RIGHT_CONTROL
    case SDLK_RALT: return 346; // GLFW_KEY_RIGHT_ALT

    // Alphanumeric keys (A-Z map directly, 0-9 map directly)
    default:
      if (sdlKey >= SDLK_a && sdlKey <= SDLK_z) {
        return 65 + (sdlKey - SDLK_a); // GLFW_KEY_A to GLFW_KEY_Z
      }
      if (sdlKey >= SDLK_0 && sdlKey <= SDLK_9) {
        return 48 + (sdlKey - SDLK_0); // GLFW_KEY_0 to GLFW_KEY_9
      }
      return sdlKey;
  }
}

int Window::mapSDL2MouseButtonToGLFW(int sdlButton) {
  // Map SDL2 mouse buttons to GLFW mouse buttons
  switch (sdlButton) {
    case SDL_BUTTON_LEFT: return 0;   // GLFW_MOUSE_BUTTON_LEFT
    case SDL_BUTTON_RIGHT: return 1;  // GLFW_MOUSE_BUTTON_RIGHT
    case SDL_BUTTON_MIDDLE: return 2; // GLFW_MOUSE_BUTTON_MIDDLE
    default: return sdlButton - 1;
  }
}

void Window::initMousePos()
{
  int mousex, mousey;
  SDL_GetMouseState(&mousex, &mousey);
  setMousePos(mousex, mousey);
}
