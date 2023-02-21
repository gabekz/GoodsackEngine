/* File: shader.c

   Shader function' implmenetations.

*/

#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <core/api/device.h>
#include <util/logger.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#ifdef SYS_ENV_WIN
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>

// WIN ONLY {TODO}
static FILE *
open_memstream(char **buffer, int bufferLen)
{

    FILE *stream;

    /* create tmp file and get file descriptor */
    int fd;
    stream = tmpfile();
    fd     = fileno(stream);

#ifdef WIN32
    HANDLE fm;
    HANDLE h = (HANDLE)_get_osfhandle(fd);

    fm =
      CreateFileMapping(h, NULL, PAGE_READWRITE | SEC_RESERVE, 0, 16384, NULL);
    if (fm == NULL) {
        fprintf(stderr,
                "%s: Couldn't access memory space!\n",
                strerror(GetLastError()));
        exit(GetLastError());
    }
    *buffer = (char *)MapViewOfFile(fm, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (*buffer == NULL) {
        fprintf(stderr,
                "%s: Couldn't fill memory space!\n",
                strerror(GetLastError()));
        exit(GetLastError());
    }
#else
    bp =
      mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, fd, 0);
    if (bp == MAP_FAILED) {
        fprintf(stderr,
                "%s: Couldn't access memory space!\n",
                FileName,
                strerror(errno));
        exit(errno);
    }
#endif

    /* return stream that is now buffer-mapped */
    return stream;
}

#endif // SYS_ENV_WIN {TODO}

/* Compile single shader type (vertex, fragment, etc.) and return
 * the id from OpenGL.
 */
static unsigned int
CompileSingleShader(unsigned int type, char *path)
{
    unsigned int id = glCreateShader(type);
    // const char* src = &source[0];
    glShaderSource(id, 1, &path, NULL);
    glCompileShader(id);

    /* Error handling */
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    const char *typeStr = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        // printf("Error at: %s\n", path);
        printf("Failed to compile %s shader.\n Error output: %s\n",
               typeStr,
               message);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

/* Parse shader source. As per current spec, this can contain both vertex
 * and fragment shaders in one file.
 * Returns the source object containing id's and compiled sources.
 */
static ShaderSource *
ParseShader(const char *path)
{
    // File in
    FILE *fptr = NULL;
    char line[256];

    // output stream
    FILE *stream = NULL;
    char *vertOut, *fragOut, *compOut;
    size_t vertLen = 8, fragLen = 1, compLen = 0;

    short mode = -1; /* -1: NONE | 0: Vert | 1: Frag */

    if ((fptr = fopen(path, "rb")) == NULL) {
        printf("Error opening %s\n", path);
        exit(1);
    }

    while (fgets(line, sizeof(line), fptr)) {
        // Line defines shader type
        if (strstr(line, "#shader") != NULL) {
            if (stream != NULL) {
                // Close stream for restart
                fflush(stream);
                rewind(stream);
                // fclose(stream);
                /*
                if (fclose(stream)) {
                    printf("Failed to close stream.");
                    exit(1);
                }
                */
            }

            // Begin vertex
            if (strstr(line, "vertex") != NULL) {
                mode   = 0;
                stream = open_memstream(&vertOut, vertLen);
            }
            // Begin fragment
            else if (strstr(line, "fragment") != NULL) {
                mode   = 1;
                stream = open_memstream(&fragOut, fragLen);
            }
            // Begin Compute
            else if (strstr(line, "compute") != NULL) {
                mode    = 2;
                stream  = open_memstream(&compOut, compLen);
                compLen = 1;
            } else {
                mode = -1;
            } // Currently no other modes
        } else {
            if (mode > -1) {
                // fread(vertOut, ftell(fptr), 1, fptr);
                fprintf(stream, line);
            }
        }
    }

    // rewind(stream);
    //  Note: fclose() also flushes the stream
    // fclose(stream);
    fflush(stream);
    rewind(stream);
    fclose(stream);

    /* TODO: Report 'NULL' declaration bug */

    ShaderSource *ss = malloc(sizeof(ShaderSource));

    // TODO: Is this malloc'd?
    if (vertLen > 0) {
        ss->shaderVertex = strdup(vertOut);
        // free(vertOut);
    }
    if (fragLen > 0) {
        ss->shaderFragment = strdup(fragOut);
        // free(fragOut);
    }
    if (compLen > 0) {
        ss->shaderCompute = strdup(compOut);
        // free(compOut);
    }

    return ss;
}

ShaderProgram *
shader_create_program(const char *path)
{
    if (DEVICE_API_OPENGL) {
        ShaderSource *ss = ParseShader(path);

        ui32 program = glCreateProgram();
        ui32 vs      = CompileSingleShader(GL_VERTEX_SHADER, ss->shaderVertex);
        ui32 fs = CompileSingleShader(GL_FRAGMENT_SHADER, ss->shaderFragment);

        // TODO: Read documentation on these functions
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        ShaderProgram *ret = malloc(sizeof(ShaderProgram));
        ret->id            = program;
        ret->shaderSource  = ss;
        return ret;

    } else if (DEVICE_API_VULKAN) {
        LOG_DEBUG("Shader not implemented for Vulkan");
        return NULL;
    }
    return NULL;
}

ShaderProgram *
shader_create_compute_program(const char *path)
{
    ShaderSource *ss = ParseShader(path);
    ui32 program     = glCreateProgram();
    ui32 csSingle = CompileSingleShader(GL_COMPUTE_SHADER, ss->shaderCompute);

    glAttachShader(program, csSingle);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(csSingle);

    ShaderProgram *ret = malloc(sizeof(ShaderProgram));
    ret->id            = program;
    ret->shaderSource  = ss;
    return ret;
}

void
shader_use(ShaderProgram *shader)
{
    glUseProgram(shader->id);
}

void
shader_uniform(ShaderProgram *shader,
               const char *uniform,
               ui32 type,
               void *data)
{
    ui32 location = glGetUniformLocation(shader->id, uniform);

    /*
    switch(type) {
        case SI32:
            glUniform1i(location, *(int *)data);
            break;
        case FLOAT:
            glUniform1f(location, *(float *)data);
            break;
        case MAT4:
            glUniformMatrix4fv(location, 1, GL_FALSE, (float *)data);
            break;
    }
    */
}
