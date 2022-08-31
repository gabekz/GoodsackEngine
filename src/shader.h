#ifndef _SHADER_
#define _SHADER_

#include <util/sysdefs.h>
#include "gfx.h"

typedef struct _shaderSource ShaderSource;

struct _shaderSource 
{
   char* shaderVertex;
   char* shaderFragment;
};

ShaderSource *ParseShader(const char *path);
unsigned int CreateShader(const char *source);

#endif
