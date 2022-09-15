#ifndef H_DEBUG
#define H_DEBUG

#include "../gfx.h"
#include<stdio.h>
#include<stdlib.h>

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam);

void glDebugInit();

#endif
