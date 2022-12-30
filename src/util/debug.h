#ifndef H_DEBUG
#define H_DEBUG

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
_error_callback(int error, const char *description);

void
GLClearError();
void
GLCheckError();

#endif
