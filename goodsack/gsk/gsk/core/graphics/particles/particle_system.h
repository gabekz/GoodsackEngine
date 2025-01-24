/*
 * Copyright (c) 2025-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_PARTICLE_SYSTEM_H__
#define __GSK_PARTICLE_SYSTEM_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/graphics/mesh/mesh.h"
#include "core/graphics/shader/shader.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_Particle
{
    vec4 startPos;
    vec4 position;
    vec4 velocity;
    vec4 convergenceTarget;
    f32 life;
    f32 colorLookup;
    f32 STRIDE_FILLER1;
    f32 STRIDE_FILLER2;
} gsk_Particle;

#define _GSK_PARTICLE_SIZE 80

#define _GSK_PARTICLE_COUNT 10000
#define _GSK_PARTICLE_AWAY  99999999.0f;

void
gsk_particle_system_init(gsk_ShaderProgram *p_compute_shader,
                         gsk_ShaderProgram *p_render_shader,
                         gsk_MeshData *p_emitter_mesh);

void
gsk_particle_system_update(gsk_ShaderProgram *p_compute_shader,
                           gsk_ShaderProgram *p_render_shader);

void
gsk_particle_system_render(gsk_ShaderProgram *p_render_shader);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_PARTICLE_SYSTEM_H__
