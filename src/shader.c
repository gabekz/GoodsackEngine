/* File: shader.c
   
   Shader function' implmenetations.

*/
#include "shader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static unsigned int CompileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    //const char* src = &source[0];
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    /* Error handling */ 
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); 
    if(result == GL_FALSE) {
       int length;
       glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
       char* message = (char *)alloca(length * sizeof(char));
       glGetShaderInfoLog(id, length, &length, message);
       printf("Failed to compile shader.\n Error output: %s\n", message);
       glDeleteShader(id);
       return 0;
    }
    return id;
}


ShaderSource *ParseShader(const char *path) {
   // File in 
   FILE* fptr = NULL;
   char line[256];

   // output stream
   FILE* stream = NULL;
   char *vertOut, *fragOut;
   size_t vertLen, fragLen;
   
   short mode = -1; /* -1: NONE | 0: Vert | 1: Frag */

   //MyShaderStruct ss;
   //ss.a = 3;

   if((fptr = fopen(path, "rb")) == NULL)
   {
      printf("Error opening %s\n", path);
      exit(1);
   }

   while(fgets(line, sizeof(line), fptr))
   {
      // Line defines shader type
      if(strstr(line, "#shader") != NULL)
      {
         if(stream != NULL)
         {
            // Close stream for restart
            if(fclose(stream))
            {
               printf("Failed to close stream.");
               exit(1);
            }
         }

         // Begin vertex
         if(strstr(line, "vertex") != NULL)
         {
            mode = 0;
            stream = open_memstream(&vertOut, &vertLen);

         }
         // Begin fragment
         else if(strstr(line, "fragment") != NULL)
         {
            //char* newOut;
            //fragOut = newOut;
            mode = 1;
            stream = open_memstream(&fragOut, &fragLen);
         }
         else { mode = -1; } // Currently no other modes
      }
      else
      {
         if(mode > -1)
            fprintf(stream, line);
      }
   }

   // Note: fclose() also flushes the stream
   fclose(stream);
   fclose(fptr);

   /* TODO: Report 'NULL' declaration bug */

   ShaderSource *ss = malloc(sizeof(ShaderSource));

   // TODO: Is this malloc'd?
   ss->shaderVertex   = strdup(vertOut);
   ss->shaderFragment = strdup(fragOut);

   free(vertOut);
   free(fragOut);

   return ss;
}


unsigned int CreateShader(const char *source) {
    ShaderSource *ss = ParseShader(source);

    ui32 program = glCreateProgram();   
    ui32 vs = CompileShader(GL_VERTEX_SHADER, ss->shaderVertex);
    ui32 fs = CompileShader(GL_FRAGMENT_SHADER, ss->shaderFragment);

    // TODO: Read documentation on these functions
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}
