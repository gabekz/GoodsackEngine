#ifndef _SHADER_
#define _SHADER_

#include <util/sysdefs.h>
#include "gfx.h"

typedef struct _shaderProgram   ShaderProgram;
typedef struct _shaderSource    ShaderSource;

struct _shaderProgram {
    ui32 id;
    ShaderSource *shaderSource;
};

struct _shaderSource {
   char *shaderVertex, *shaderFragment;
};

ShaderProgram *shader_create_program(const char *path);
//void shader_uniform(ShaderProgram *shader, ui32 type, void* data);
void shader_use(ShaderProgram *shader);

#endif
