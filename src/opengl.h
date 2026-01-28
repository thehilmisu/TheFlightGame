#ifndef OPENGL_H
#define OPENGL_H

// Platform-agnostic OpenGL header
// Replaces GLAD for SDL2-based builds

#ifdef __ANDROID__
  // Android uses OpenGL ES 3.0
  #include <GLES3/gl3.h>
#else
  // Desktop uses OpenGL via SDL2
  #define GL_GLEXT_PROTOTYPES 1
  #include <SDL_opengl.h>
#endif

#endif // OPENGL_H
