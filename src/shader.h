#ifndef _SHADER_
#define _SHADER_

#include "gfx.h"

struct myShaderStruct 
{
   char* shaderVertex;
   char* shaderFragment;
};
typedef struct myShaderStruct MyShaderStruct;

MyShaderStruct* ParseShader(const char* path);
unsigned int CompileShader(unsigned int type, const char* source);
unsigned int CreateShader(char * vertexShader, char* fragmentShader);


#endif
