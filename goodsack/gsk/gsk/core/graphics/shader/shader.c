/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/device/device.h"

#ifdef SYS_ENV_WIN
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>
#endif // SYS_ENV_WIN

// Windows-only function
// TODO: possibly move to util/filesystem.h
#ifdef SYS_ENV_WIN

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
    if (fm == NULL)
    {
        fprintf(stderr,
                "%s: Couldn't access memory space!\n",
                strerror(GetLastError()));
        exit(GetLastError());
    }
    *buffer = (char *)MapViewOfFile(fm, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (*buffer == NULL)
    {
        fprintf(stderr,
                "%s: Couldn't fill memory space!\n",
                strerror(GetLastError()));
        exit(GetLastError());
    }
#else
    bp =
      mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, fd, 0);
    if (bp == MAP_FAILED)
    {
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

#endif

/* Compile single shader type (vertex, fragment, etc.) and return
 * the id from OpenGL.
 */
static unsigned int
CompileSingleShader(unsigned int type, const char *path)
{
    unsigned int id = glCreateShader(type);
    // const char* src = &source[0];
    glShaderSource(id, 1, &path, NULL);
    glCompileShader(id);

    /* Error handling */
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    const char *typeStr = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
    if (result == GL_FALSE)
    {
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
static gsk_ShaderSource *
ParseShader(const char *path)
{
    // File in
    FILE *fptr = NULL;
    char line[256];

    // output stream
    FILE *stream = NULL;
    char *vertOut, *fragOut, *compOut;
    size_t vertLen = 0, fragLen = 0, compLen = 0;

    short mode = -1; /* -1: NONE | 0: Vert | 1: Frag */

    if ((fptr = fopen(path, "rb")) == NULL)
    {
        LOG_ERROR("Error opening %s\n", path);
        exit(1);
    }

    while (fgets(line, sizeof(line), fptr))
    {
        // Line defines shader type
        if (strstr(line, "#shader") != NULL)
        {
            if (stream != NULL)
            {
                // Close stream for restart
                //
#ifdef WIN32
                fflush(stream);
                rewind(stream);
#else
                if (fclose(stream))
                {
                    LOG_ERROR("Failed to close stream.");
                    exit(1);
                }
#endif
            }

            // Begin vertex
            if (strstr(line, "vertex") != NULL)
            {
                mode = 0;
#ifdef WIN32
                stream  = open_memstream(&vertOut, vertLen);
                vertLen = 1;
#else
                stream = open_memstream(&vertOut, &vertLen);
#endif
            }
            // Begin fragment
            else if (strstr(line, "fragment") != NULL)
            {
                mode = 1;
#ifdef WIN32
                stream  = open_memstream(&fragOut, fragLen);
                fragLen = 1;
#else
                stream = open_memstream(&fragOut, &fragLen);
#endif
            }
            // Begin Compute
            else if (strstr(line, "compute") != NULL)
            {
                mode = 2;
#ifdef WIN32
                stream  = open_memstream(&compOut, compLen);
                compLen = 1;
#else
                stream = open_memstream(&compOut, &compLen);
#endif
                // compLen = 1;
            } else
            {
                mode = -1;
            } // Currently no other modes
        } else
        {
            if (mode > -1)
            {
                // fread(vertOut, ftell(fptr), 1, fptr);
                fprintf(stream, line);
            }
        }
    }

    if (stream != NULL) fclose(stream);
    if (fptr != NULL) fclose(fptr);

    /* TODO: Report 'NULL' declaration bug */

    gsk_ShaderSource *ss = malloc(sizeof(gsk_ShaderSource));

    // TODO: Is this malloc'd?
    if (vertLen > 0) { ss->shaderVertex = strdup(vertOut); }
    if (fragLen > 0) { ss->shaderFragment = strdup(fragOut); }
    if (compLen > 0) { ss->shaderCompute = strdup(compOut); }

    return ss;
}

gsk_ShaderProgram *
gsk_shader_program_create(const char *path)
{
    if (GSK_DEVICE_API_OPENGL)
    {
        gsk_ShaderSource *ss = ParseShader(path);

        u32 program = glCreateProgram();
        u32 vs      = CompileSingleShader(GL_VERTEX_SHADER, ss->shaderVertex);
        u32 fs = CompileSingleShader(GL_FRAGMENT_SHADER, ss->shaderFragment);

        // TODO: Read documentation on these functions
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        gsk_ShaderProgram *ret = malloc(sizeof(gsk_ShaderProgram));
        ret->id                = program;
        ret->shaderSource      = ss;
        return ret;

    } else if (GSK_DEVICE_API_VULKAN)
    {
        LOG_DEBUG("Shader not implemented for Vulkan");
        return NULL;
    }
    return NULL;
}

gsk_ShaderProgram *
gsk_shader_compute_program_create(const char *path)
{
    gsk_ShaderSource *ss = ParseShader(path);
    u32 program          = glCreateProgram();
    u32 csSingle = CompileSingleShader(GL_COMPUTE_SHADER, ss->shaderCompute);

    glAttachShader(program, csSingle);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(csSingle);

    gsk_ShaderProgram *ret = malloc(sizeof(gsk_ShaderProgram));
    ret->id                = program;
    ret->shaderSource      = ss;
    return ret;
}

void
gsk_shader_use(gsk_ShaderProgram *shader)
{
    glUseProgram(shader->id);
}

#if _GSK_SHADER_EASY_UNIFORMS
void
gsk_shader_uniform(gsk_ShaderProgram *shader,
                   const char *uniform,
                   u32 type,
                   void *data)
{
    u32 location = glGetUniformLocation(shader->id, uniform);

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
#endif // _GSK_SHADER_EASY_UNIFORMS
