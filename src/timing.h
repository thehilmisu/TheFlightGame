#ifndef TIMING_H
#define TIMING_H

#include <SDL.h>

//Upper bound on the delta time a single frame is allowed to report.
//Without it, any stall (alt-tab, a chunk generation hitch, a breakpoint)
//produces one enormous step: the plane is integrated hundreds of units in a
//single frame and tunnels straight through terrain and collision volumes.
//Capping it makes a stalled frame play in slow motion instead, which is the
//standard trade and is invisible at these frame times.
constexpr float MAX_DT = 0.1f;

// get time in seconds
inline double getTime() {
  //SDL_GetTicks() only has millisecond resolution, which quantises dt badly at
  //high frame rates. The performance counter is monotonic and sub-microsecond.
  static const Uint64 frequency = SDL_GetPerformanceFrequency();
  return static_cast<double>(SDL_GetPerformanceCounter()) /
         static_cast<double>(frequency);
}

//Turns a measured frame duration into a delta time the simulation can safely
//integrate. Negative input cannot happen with a monotonic clock but is folded
//to zero rather than trusted.
inline float clampDt(double measured) {
  if (measured < 0.0)
    return 0.0f;
  return static_cast<float>(measured < MAX_DT ? measured : MAX_DT);
}

#endif // TIMING_H
