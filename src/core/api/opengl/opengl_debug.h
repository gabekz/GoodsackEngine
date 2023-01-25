#ifndef H_OPENGL_DEBUG
#define H_OPENGL_DEBUG

#include <util/gfx.h>

void APIENTRY
glDebugOutput(GLenum source,
              GLenum type,
              unsigned int id,
              GLenum severity,
              GLsizei length,
              const char *message,
              const void *userParam);

void
glDebugInit();
// ~~~
void
_error_callback_gl(int error, const char *description);

void
GLClearError();
void
GLCheckError();

#endif // H_OPENGL_DEBUG
