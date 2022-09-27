/* File: shader.c
   
   Shader function' implmenetations.

*/
#include "../shader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Compile single shader type (vertex, fragment, etc.) and return
 * the id from OpenGL.
 */ 
static unsigned int CompileSingleShader(unsigned int type, const char* path) {
    unsigned int id = glCreateShader(type);
    //const char* src = &source[0];
    glShaderSource(id, 1, &path, NULL);
    glCompileShader(id);

    /* Error handling */ 
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); 
    const char *typeStr = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
    if(result == GL_FALSE) {
       int length;
       glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
       char* message = (char *)alloca(length * sizeof(char));
       glGetShaderInfoLog(id, length, &length, message);
       printf("Error at: %s\n", path);
       printf("Failed to compile %s shader.\n Error output: %s\n", typeStr, message);
       glDeleteShader(id);
       return 0;
    }
    return id;
}

/* Parse shader source. As per current spec, this can contain both vertex
 * and fragment shaders in one file.
 * Returns the source object containing id's and compiled sources.
 */ 
static ShaderSource *ParseShader(const char *path) {
   // File in 
   FILE* fptr = NULL;
   char line[256];

   // output stream
   FILE* stream = NULL;
   char *vertOut, *fragOut;
   size_t vertLen, fragLen;
   
   short mode = -1; /* -1: NONE | 0: Vert | 1: Frag */

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


ShaderProgram *shader_create_program(const char *path) {
    ShaderSource *ss = ParseShader(path);

    ui32 program = glCreateProgram();   
    ui32 vs = CompileSingleShader(GL_VERTEX_SHADER, ss->shaderVertex);
    ui32 fs = CompileSingleShader(GL_FRAGMENT_SHADER, ss->shaderFragment);

    // TODO: Read documentation on these functions
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    ShaderProgram *ret = malloc(sizeof(ShaderProgram)); 
    ret->id = program;
    ret->shaderSource = ss;
    return ret;
}

void shader_use(ShaderProgram *shader) {
    glUseProgram(shader->id);
}
