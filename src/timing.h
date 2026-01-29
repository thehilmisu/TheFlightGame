#ifndef TIMING_H
#define TIMING_H

#include <SDL.h>

// get time in seconds
inline double getTime() {
  return SDL_GetTicks() / 1000.0;
}

#endif // TIMING_H
