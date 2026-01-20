#include "window.h"
#include "GLFW/glfw3.h"
#include <iostream>

Window::Window(){

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  mWindow = glfwCreateWindow(mWidth, mHeight, mTitle.data(), NULL, NULL);
  if (!mWindow) {
    std::cerr << "Failed to create GLFW window";
    glfwTerminate();
    return;
  }
	glfwMakeContextCurrent(mWindow);
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
	glfwSetKeyCallback(mWindow, handleKeyInput);
	glfwSetCursorPosCallback(mWindow, cursorPosCallback);
	glfwSetMouseButtonCallback(mWindow, handleMouseInput);
	glfwSetScrollCallback(mWindow, scrollCallback);
  glfwMakeContextCurrent(mWindow);

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD";
  }

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
  if(mWindow) {
    glfwDestroyWindow(mWindow);
  }
  glfwTerminate();
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height){
  (void)window;
  getInstance().onResize(width, height);
}

void Window::handleWindowResizeImpl(GLFWwindow* window, int w, int h){
  (void)window;
  onResize(w, h);
}

void Window::handleWindowResize(GLFWwindow* window, int w, int h){
  getInstance().handleWindowResizeImpl(window, w, h);
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

void Window::cursorPosCallback(GLFWwindow* window, double x, double y)
{
  (void)window;
	// int cursorMode = glfwGetInputMode(mWindow, GLFW_CURSOR);
	double dx = x - getMouseX();
	double dy = y - getMouseY();
	setMousePos(x, y);
	setMouseDiff(dx, dy);
}

void Window::handleKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  (void)window;
	if(action == GLFW_PRESS)
		setKey(key, JUST_PRESSED);
	else if(action == GLFW_RELEASE)
		setKey(key, RELEASED);
}

void Window::handleMouseInput(GLFWwindow* window, int button, int action, int mods)
{
  (void)window;
	if(action == GLFW_PRESS)
		setButton(button, JUST_PRESSED);
	else if(action == GLFW_RELEASE)
		setButton(button, RELEASED);
}

void Window::scrollCallback(GLFWwindow* window,double xoff, double yoff)
{
  (void)window;
	setScrollSpeed(yoff);
}

void Window::initMousePos()
{
	double mousex, mousey;
	glfwGetCursorPos(mWindow, &mousex, &mousey);
	setMousePos(mousex, mousey);
}

bool Window::keyIsHeld(KeyState keystate)
{
	return keystate == JUST_PRESSED || keystate == HELD;
}

void Window::setCursorInputMode(int mode){
	glfwSetInputMode(mWindow, GLFW_CURSOR, mode);
}                
            
      
    
