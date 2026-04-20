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
#include "runtime/gsk_runtime_wrapper.h"

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
CompileSingleShader(unsigned int type,
                    const char *raw_shader_text,
                    u8 is_skinned)
{
    const char *versionLine = "#version 460 core\n";
    const char *defineLine =
      is_skinned ? "#define SKINNED 1\n" : "#define SKINNED 0\n";

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
gsk_ShaderSource
gsk_shader_source_parse(const char *path, u8 skip_version)
{
    // File in
    FILE *fptr = NULL;
    char line[1024];

    s16 mode = -1; /* -1: NONE | 0: Vert | 1: Frag | 2: Geometry | 3: Compute */

    ArrayList list_outputs[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++)
    {
        list_outputs[i] = LIST_INIT(sizeof(char), 20);
    }

    if ((fptr = fopen(path, "rb")) == NULL)
    {
        LOG_CRITICAL("Error opening %s\n", path);
    }

    while (fgets(line, sizeof(line), fptr))
    {
        // Line defines shader type
        if (strstr(line, "#shader") != NULL)
        {
            // Begin vertex
            if (strstr(line, "vertex") != NULL)
            {
                mode = 0;
            }
            // Begin fragment
            else if (strstr(line, "fragment") != NULL)
            {
                mode = 1;
            }
            // Begin Geometry
            else if (strstr(line, "geometry") != NULL)
            {
                mode = 2;
            }
            // Begin Compute
            else if (strstr(line, "compute") != NULL)
            {
                mode = 3;
            }
            // no mode
            else
            {
                mode = -1;
            }
        }
        // skip "#version" lines to fill in our own
        else if (skip_version && strstr(line, "#version") != NULL)
        {
            continue;
        }
        // write line
        else if (mode > -1)
        {
            LIST_APPEND(&list_outputs[mode], &line, strlen(line));
        }
    }

    if (fptr != NULL) fclose(fptr);

    gsk_ShaderSource ss = {0};

    for (int i = 0; i < 4; i++)
    {
        if (list_outputs[i].is_list_empty == TRUE) { continue; }

        size_t headerLen = 0;
        size_t buff_len  = list_outputs[i].list_next + headerLen + 1;

        char *p_str = malloc(buff_len);
        snprintf(p_str, buff_len, "%s", list_outputs[i].data.buffer);

        array_list_free(&list_outputs[i]);

        // TODO: clean this up.
        if (i == 0)
        {
            ss.shaderVertex = p_str;
            ss.len_vertex   = buff_len;
        } else if (i == 1)
        {
            ss.shaderFragment = p_str;
            ss.len_fragment   = buff_len;
        } else if (i == 2)
        {
            ss.shaderGeometry = p_str;
            ss.len_geometry   = buff_len;
        } else if (i == 3)
        {
            ss.shaderCompute = p_str;
            ss.len_compute   = buff_len;
        }
    }

    return ss;
}

gsk_ShaderProgram
gsk_shader_program_import_from_file(const char *path)
{
    if (GSK_DEVICE_API_VULKAN)
    {
        LOG_DEBUG("Shader not implemented for Vulkan");
        gsk_ShaderProgram ret = {.id = 0, .shaderSource = NULL};
        return ret;
    }

    gsk_ShaderSource ss   = gsk_shader_source_parse(path, TRUE);
    gsk_ShaderProgram ret = {.id = 0, .id_skinned = 0, .shaderSource = ss};
    return ret;
}

u8
gsk_shader_program_load(gsk_ShaderProgram *p_self)
{
    if (GSK_DEVICE_API_VULKAN)
    {
        LOG_DEBUG("Shader not implemented for Vulkan");
        gsk_ShaderProgram ret = {.id = 0, .shaderSource = NULL};
        return FALSE;
    }

    gsk_ShaderSource ss = p_self->shaderSource;

    for (int i = 0; i < 2; i++)
    {
        u32 program = glCreateProgram();

        u32 vs = (ss.len_vertex > 0)
                   ? CompileSingleShader(GL_VERTEX_SHADER, ss.shaderVertex, i)
                   : 0;
        u32 fs =
          (ss.len_fragment > 0)
            ? CompileSingleShader(GL_FRAGMENT_SHADER, ss.shaderFragment, i)
            : 0;
        u32 gs =
          (ss.len_geometry > 0)
            ? CompileSingleShader(GL_GEOMETRY_SHADER, ss.shaderGeometry, i)
            : 0;
        u32 cs = (ss.len_compute > 0)
                   ? CompileSingleShader(GL_COMPUTE_SHADER, ss.shaderCompute, i)
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

        if (i == 0)
        {
            p_self->id = program;
        } else if (i == 1)
        {
            p_self->id_skinned = program;
        }
    }

    return TRUE;
}

void
gsk_shader_use(gsk_ShaderProgram *shader)
{
    _gsk_shader_use_program(shader->id);
}

u32
_gsk_shader_use_program(u32 shader_program_id)
{
    glUseProgram(shader_program_id);

    // update previous shader_id on renderer
    gsk_runtime_get_renderer()->prev_shader_id = shader_program_id;
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

void
gsk_shader_archive(GskArchiveMode archive_mode,
                   gsk_ShaderProgram *p_shader,
                   gsk_AssetBlob *p_blob)
{
    gsk_Archive archive = {
      .mode = archive_mode,
    };

    // read-mode
    if (archive_mode == GskArchiveMode_Read)
    {
        archive.p_buffer = p_blob->p_buffer;
        archive.seek_cnt = 0;
    }
    // write-mode
    else if (archive_mode == GskArchiveMode_Write)
    {
        archive.out = LIST_INIT(sizeof(u8), 20);
    }

    gsk_ShaderSource *p_shader_source = &p_shader->shaderSource;

    GPAK_ARCHIVE(&archive, &p_shader_source->len_vertex);
    GPAK_ARCHIVE(&archive, &p_shader_source->len_fragment);
    GPAK_ARCHIVE(&archive, &p_shader_source->len_geometry);
    GPAK_ARCHIVE(&archive, &p_shader_source->len_compute);

    if (archive_mode == GskArchiveMode_Read)
    {
        p_shader_source->shaderVertex   = malloc(p_shader_source->len_vertex);
        p_shader_source->shaderFragment = malloc(p_shader_source->len_fragment);
        p_shader_source->shaderGeometry = malloc(p_shader_source->len_geometry);
        p_shader_source->shaderCompute  = malloc(p_shader_source->len_compute);
    }

    gsk_archive_bytes(&archive,
                      p_shader_source->shaderVertex,
                      sizeof(char) * p_shader_source->len_vertex);

    gsk_archive_bytes(&archive,
                      p_shader_source->shaderFragment,
                      sizeof(char) * p_shader_source->len_fragment);

    gsk_archive_bytes(&archive,
                      p_shader_source->shaderGeometry,
                      sizeof(char) * p_shader_source->len_geometry);

    gsk_archive_bytes(&archive,
                      p_shader_source->shaderCompute,
                      sizeof(char) * p_shader_source->len_compute);

    if (archive_mode == GskArchiveMode_Write)
    {
        p_blob->asset_type    = GskAssetType_Shader;
        p_blob->p_buffer      = archive.out.data.buffer;
        p_blob->buffer_len    = archive.out.data.buffer_size;
        p_blob->is_serialized = TRUE;
    }
}