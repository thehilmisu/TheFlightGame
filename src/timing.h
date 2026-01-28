#ifndef TIMING_H
#define TIMING_H

#include <SDL.h>

// Helper function to get time in seconds (replaces glfwGetTime)
inline double getTime() {
  return SDL_GetTicks() / 1000.0;
}

#endif // TIMING_H
