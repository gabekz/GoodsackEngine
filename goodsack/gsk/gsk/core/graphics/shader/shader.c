/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
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
open_memstream(char **buffer, int *buffer_check)
{

    FILE *stream;

    /* create tmp file and get file descriptor */
    int fd;
    stream = tmpfile();
    fd     = fileno(stream);

    HANDLE fm;
    HANDLE h = (HANDLE)_get_osfhandle(fd);

    // TODO: max buffer size here
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
#if 0
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
#endif // disable

    // TODO: Temporary fix to get strdup working
    *buffer_check = 1;
    /* return stream that is now buffer-mapped */
    return stream;
}

#endif // SYS_ENV_WIN

/* Compile single shader type (vertex, fragment, geometry, etc.) and return
 * the id from OpenGL.
 */
static unsigned int
CompileSingleShader(unsigned int type, const char *raw_shader_text)
{
    u8 skinned = FALSE;

    const char *versionLine = "#version 460 core\n";
    const char *defineLine =
      skinned ? "#define SKINNED 1\n" : "#define SKINNED 0\n";

    const char *sources[] = {versionLine, defineLine, raw_shader_text};

    unsigned int id = glCreateShader(type);

    glShaderSource(id, 3, sources, NULL);

    glCompileShader(id);

    /* Error handling */
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        // printf("Error at: %s\n", path);
        LOG_ERROR("Failed to compile shader (type: %d).\n Error output: %s\n",
                  type,
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
static gsk_ShaderSource
ParseShader(const char *path)
{
    // File in
    FILE *fptr = NULL;
    char line[1024];

    // output stream
    FILE *stream = NULL;
    char *vertOut, *fragOut, *compOut, *geomOut;
    size_t vertLen = 0, fragLen = 0, geomLen = 0, compLen = 0;

    short mode =
      -1; /* -1: NONE | 0: Vert | 1: Frag | 2: Geometry | 3: Compute */

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
                mode   = 0;
                stream = open_memstream(&vertOut, &vertLen);
            }
            // Begin fragment
            else if (strstr(line, "fragment") != NULL)
            {
                mode   = 1;
                stream = open_memstream(&fragOut, &fragLen);
            }
            // Begin Geometry
            else if (strstr(line, "geometry") != NULL)
            {
                mode   = 2;
                stream = open_memstream(&geomOut, &geomLen);
            }
            // Begin Compute
            else if (strstr(line, "compute") != NULL)
            {
                mode   = 3;
                stream = open_memstream(&compOut, &compLen);
            } else
            {
                mode = -1;
            }
        }
        // skip "#version" lines to fill in our own
        else if (strstr(line, "#version") != NULL)
        {
            continue;
        }
        // write line
        else
        {
            if (mode > -1) { fprintf(stream, line); }
        }
    }

    if (stream != NULL) fclose(stream);
    if (fptr != NULL) fclose(fptr);

    /* TODO: Report 'NULL' declaration bug */

    gsk_ShaderSource ss = {0};

    if (vertLen > 0) { ss.shaderVertex = strdup(vertOut); }
    if (fragLen > 0) { ss.shaderFragment = strdup(fragOut); }
    if (geomLen > 0) { ss.shaderGeometry = strdup(geomOut); }
    if (compLen > 0) { ss.shaderCompute = strdup(compOut); }

    return ss;
}

gsk_ShaderProgram
gsk_shader_program_create(const char *path)
{
    if (GSK_DEVICE_API_OPENGL)
    {
        gsk_ShaderSource ss = ParseShader(path);

        u32 program = glCreateProgram();

        u32 vs = (ss.shaderVertex)
                   ? CompileSingleShader(GL_VERTEX_SHADER, ss.shaderVertex)
                   : 0;
        u32 fs = (ss.shaderFragment)
                   ? CompileSingleShader(GL_FRAGMENT_SHADER, ss.shaderFragment)
                   : 0;
        u32 gs = (ss.shaderGeometry)
                   ? CompileSingleShader(GL_GEOMETRY_SHADER, ss.shaderGeometry)
                   : 0;
        u32 cs = (ss.shaderCompute)
                   ? CompileSingleShader(GL_COMPUTE_SHADER, ss.shaderCompute)
                   : 0;

        if (vs) { glAttachShader(program, vs); }
        if (fs) { glAttachShader(program, fs); }
        if (gs) { glAttachShader(program, gs); }
        if (cs) { glAttachShader(program, cs); }

        glLinkProgram(program);

        GLint linkStatus = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            char *infoLog = (char *)malloc(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);

            // Log the error
            LOG_ERROR("Program link error message: %s", infoLog);

            free(infoLog);
        }

        glValidateProgram(program);

        if (vs) { glDeleteShader(vs); }
        if (fs) { glDeleteShader(fs); }
        if (gs) { glDeleteShader(gs); }
        if (cs) { glDeleteShader(cs); }

        gsk_ShaderProgram ret = {.id = program, .shaderSource = ss};

        return ret;

    } else if (GSK_DEVICE_API_VULKAN)
    {
        LOG_DEBUG("Shader not implemented for Vulkan");
        gsk_ShaderProgram ret = {.id = 0, .shaderSource = NULL};
        return ret;
    }
    gsk_ShaderProgram ret = {.id = 0, .shaderSource = NULL};
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
