/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __SHADER_H__
#define __SHADER_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define _GSK_SHADER_EASY_UNIFORMS 0

#if _GSK_SHADER_EASY_UNIFORMS
typedef enum UniformType {
    UNIFORM_1f = 0,
    UNIFORM_2f,
    UNIFORM_3f,
    UNIFORM_4f,

    UNIFORM_1i,
    UNIFORM_2i,
    UNIFORM_3i,
    UNIFORM_4i,

    UNIFORM_1ui,
    UNIFORM_2ui,
    UNIFORM_3ui,
    UNIFORM_4ui,

    UNIFORM_1fv,
    UNIFORM_2fv,
    UNIFORM_3fv,
    UNIFORM_4fv,

    UNIFORM_1iv,
    UNIFORM_2iv,
    UNIFORM_3iv,
    UNIFORM_4iv,

} UniformType;
#endif // _GSK_SHADER_EASY_UNIFORMS

typedef struct gsk_ShaderSource
{
    char *shaderVertex, *shaderFragment, *shaderGeometry, *shaderCompute;
} gsk_ShaderSource;

typedef struct gsk_ShaderProgram
{
    u32 id, id_skinned;
    gsk_ShaderSource shaderSource;
} gsk_ShaderProgram;

gsk_ShaderProgram
gsk_shader_program_create(const char *path);

void
gsk_shader_use(gsk_ShaderProgram *shader);

u32
_gsk_shader_use_program(u32 shader_program_id);

#if _GSK_SHADER_EASY_UNIFORMS
void
gsk_shader_uniform(gsk_ShaderProgram *shader,
                   const char *uniform,
                   u32 type,
                   void *data);
#endif // _GSK_SHADER_EASY_UNIFORMS

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SHADER_H__
