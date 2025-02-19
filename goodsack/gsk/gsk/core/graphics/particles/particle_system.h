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

typedef struct gsk_ParticleSystem
{
    f32 noise_min, noise_max;
    f32 noise_multiplier;
    f32 noise_speed;
    f32 noise_cnt;

    f32 ramp_dist;
    f32 updraft;

    f32 min_life, max_life;
    f32 size_life_min, size_life_max;

    vec3 convergence_point_world_pos;
    f32 convergence_strength;

    vec3 world_pos, world_rot, world_scale;
    mat4 model_matrix;

    s32 particle_count;

    gsk_ShaderProgram *p_compute_shader;
    gsk_ShaderProgram *p_render_shader;

    u32 ssbo_particle_id, ssbo_mesh_id;

    f32 *particles_buff;
    u32 particles_buff_size;
    f32 *mesh_buff;
    u32 mesh_buff_size;

    u32 num_verts;

} gsk_ParticleSystem;

#define _GSK_MAX_PARTICLE_COUNT 10000
#define _GSK_PARTICLE_COUNT     10000

#define _GSK_PARTICLE_AWAY 99999999.0f

void
gsk_particle_system_initialize();

gsk_ParticleSystem
gsk_particle_system_create(gsk_ShaderProgram *p_compute_shader,
                           gsk_ShaderProgram *p_render_shader,
                           gsk_MeshData *p_emitter_mesh);

void
gsk_particle_system_update(gsk_ParticleSystem *p_particle_system);

void
gsk_particle_system_render(gsk_ParticleSystem *p_particle_system);

void
gsk_particle_system_cleanup(gsk_ParticleSystem *p_particle_system);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_PARTICLE_SYSTEM_H__
