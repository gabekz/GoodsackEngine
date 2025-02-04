/*
 * Copyright (c) 2025-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "particle_system.h"

#include "util/filesystem.h"
#include "util/gfx.h"
#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "asset/asset.h"
#include "core/device/device.h"
#include "core/drivers/opengl/opengl.h"
#include "core/graphics/shader/shader.h"

#include <stdlib.h>

#define WARP_SIZE 1024.0f

static gsk_ShaderProgram *s_saved_render_shader;
static gsk_ShaderProgram *s_saved_compute_shader;

static f32 s_curl_E_min        = 0.1f;
static f32 s_curl_E_max        = 3.3f;
static f32 s_curl_E_multiplier = 1.05f;
static f32 s_curl_E_speed      = 8.0f;
static f32 s_curl_E            = 0;

static f32 s_smoke_dist = 1.2f;

static f32 s_min_life = 0.2f;
static f32 s_max_life = 2.0f;

static f32 s_updraft = 0.020f;

static f32 s_size_life_min = 0.05f;
static f32 s_size_life_max = 0.2f;

static u32 s_num_vert = 2;

f32
_update_curl(f32 curl_min, f32 curl_max, f32 curl_speed)
{
    return glm_lerp(curl_min,
                    curl_max,
                    sin(gsk_device_getTime().time_elapsed * curl_speed) / 2.0f +
                      0.5f);
}

void
gsk_particle_system_initialize()
{
    s_saved_compute_shader = gsk_shader_compute_program_create(
      GSK_PATH("zhr://shaders/fire_particles.compute"));

    s_saved_render_shader =
      GSK_ASSET("zhr://shaders/particles_computed.shader");
}

gsk_ParticleSystem
gsk_particle_system_create(gsk_ShaderProgram *p_compute_shader,
                           gsk_ShaderProgram *p_render_shader,
                           gsk_MeshData *p_emitter_mesh)
{
    // s_saved_render_shader = ;

    gsk_Particle *sp_particles =
      malloc(sizeof(gsk_Particle) * _GSK_MAX_PARTICLE_COUNT);

    // initialize the particles
    const u32 particle_count = _GSK_MAX_PARTICLE_COUNT;

    for (int i = 0; i < particle_count; i++)
    {
        sp_particles[i].position[0] = _GSK_PARTICLE_AWAY;
        sp_particles[i].position[1] = _GSK_PARTICLE_AWAY;
        sp_particles[i].position[2] = _GSK_PARTICLE_AWAY;

        sp_particles[i].velocity[0] = 0;
        sp_particles[i].velocity[1] = 0;
        sp_particles[i].velocity[2] = 0;

        // Initial life value
        sp_particles[i].life = ((double)rand() / (double)RAND_MAX);
    }

    // create SSBO's

    // s_mesh_ssbo_id     = 0;
    // s_particle_ssbo_id = 0;
    u32 ssbo_size    = 0;
    u32 ssbo_binding = 0;

    // Create mesh SSBO

    float *buff;

    u32 triangle_count = 2;
    vec4 triangle[6]   = {{0, 0, 0, 8},
                        {0, 0, 1, 9},
                        {1, 0, 1, 10},
                        {2, 0, 2, 8},
                        {2, 0, 0, 9},
                        {0, 0, 2, 10}};
    if (p_emitter_mesh == NULL)
    {
        LOG_WARN("Using default particle triangles");
    }
    // emitter mesh
    else
    {
        // get triangle positions from meshdata

        gsk_MeshBuffer *p_selected_mesh_buff = NULL;
        GskMeshBufferFlags flags             = 0;

        for (int i = 0; i < p_emitter_mesh->mesh_buffers_count; i++)
        {
            if (p_emitter_mesh->mesh_buffers[i].buffer_flags &
                GskMeshBufferFlag_Positions)
            {
                p_selected_mesh_buff = &(p_emitter_mesh->mesh_buffers[i]);
                flags                = p_selected_mesh_buff->buffer_flags;
                break;
            }
        }

        if (p_selected_mesh_buff == NULL)
        {
            LOG_CRITICAL("Failed to mesh buffer for particle system.");
        }

        triangle_count = p_selected_mesh_buff->buffer_size / sizeof(float);

        s_num_vert = triangle_count / 3;

        u32 triangle_len = 0;
        u32 vertex_len   = 0;
        u32 iters        = 0;

        float *p = malloc(triangle_count * (sizeof(vec4) * 3));
        buff     = p;

        // calculate triangle_len
        {
            if (flags & GskMeshBufferFlag_Positions) { vertex_len += 3; }
            if (flags & GskMeshBufferFlag_Textures) { vertex_len += 2; }
            if (flags & GskMeshBufferFlag_Normals) { vertex_len += 3; }
            if (flags & GskMeshBufferFlag_Tangents) { vertex_len += 3; }
            if (flags & GskMeshBufferFlag_Bitangents) { vertex_len += 3; }
            if (flags & GskMeshBufferFlag_Joints) { vertex_len += 4; }
            if (flags & GskMeshBufferFlag_Weights) { vertex_len += 4; }

            if (vertex_len == 0)
            {
                LOG_CRITICAL(
                  "Cannot create particle emitter on empty vertex data.");
            }

            triangle_len = vertex_len * 3;
        }

        for (int i = 0; i < triangle_count; i += triangle_len)
        {
            vec3 tri[3] = {
              GLM_VEC3_ZERO_INIT, GLM_VEC3_ZERO_INIT, GLM_VEC3_ZERO_INIT};

            float *p_buff_vert = p_selected_mesh_buff->p_buffer;

            // get only the positions
            glm_vec3_copy((float *)&p_buff_vert[i], tri[0]);
            glm_vec3_copy((float *)&p_buff_vert[i + vertex_len], tri[1]);
            glm_vec3_copy((float *)&p_buff_vert[i + (vertex_len * 2)], tri[2]);

#if 0
            LOG_INFO("POS: {%f, %f, %f}", tri[0][0], tri[0][1], tri[0][2]);
            LOG_INFO("POS: {%f, %f, %f}", tri[1][0], tri[1][1], tri[1][2]);
            LOG_INFO("POS: {%f, %f, %f}", tri[2][0], tri[2][1], tri[2][2]);
#endif

            glm_vec3_copy(tri[0], (float *)&buff[iters]);
            buff[iters + 3] = 1.0f;
            glm_vec3_copy(tri[1], (float *)&buff[iters + 4]);
            buff[iters + 7] = 1.0f;
            glm_vec3_copy(tri[2], (float *)&buff[iters + 8]);
            buff[iters + 11] = 1.0f;

            iters += 12;
        }
    }

#if 0
    LOG_TRACE("TRIANGLES: %d", triangle_count);
#endif

    u32 s_mesh_ssbo_id     = 0;
    u32 s_particle_ssbo_id = 0;

    ssbo_size    = sizeof(vec4) * 3;
    ssbo_binding = 0;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

#if 1
    glGenBuffers(1, &s_mesh_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_mesh_ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 ssbo_size * triangle_count,
                 buff,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, s_mesh_ssbo_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Create particle SSBO

    ssbo_size    = _GSK_PARTICLE_SIZE;
    ssbo_binding = 1;

    glGenBuffers(1, &s_particle_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_particle_ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 ssbo_size * _GSK_MAX_PARTICLE_COUNT,
                 sp_particles,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, s_particle_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif

    // return

    gsk_ParticleSystem ret = {
        .noise_min        = s_curl_E_min,
        .noise_max        = s_curl_E_max,
        .noise_multiplier = s_curl_E_multiplier,
        .noise_speed      = s_curl_E_speed,
        .noise_cnt        = 0,

        .ramp_dist = s_smoke_dist,
        .updraft   = s_updraft,

        .min_life = s_min_life,
        .max_life = s_max_life,

        .size_life_min = s_size_life_min,
        .size_life_max = s_size_life_max,

        .particle_count = _GSK_PARTICLE_COUNT,

        .world_pos   = {0, 0, 0},
        .world_rot   = {0, 0, 0},
        .world_scale = {1, 1, 1},

        .convergence_point_world_pos = {0, 4, 0},
        .convergence_strength        = 0.002f,

#if 1
        .p_compute_shader = s_saved_compute_shader,
        .p_render_shader  = s_saved_render_shader,
#else
        .p_compute_shader = p_compute_shader,
        .p_render_shader  = p_render_shader,
#endif

        .ssbo_mesh_id     = s_mesh_ssbo_id,
        .ssbo_particle_id = s_particle_ssbo_id,

        .particles_buff      = sp_particles,
        .mesh_buff           = buff,
        .mesh_buff_size      = (sizeof(vec4) * 3) * triangle_count,
        .particles_buff_size = _GSK_PARTICLE_SIZE * _GSK_MAX_PARTICLE_COUNT,
    };

    return ret;
}

void
gsk_particle_system_update(gsk_ParticleSystem *p_particle_system)
{

    p_particle_system->noise_cnt = _update_curl(p_particle_system->noise_min,
                                                p_particle_system->noise_max,
                                                p_particle_system->noise_speed);

    const u32 num_particles     = p_particle_system->particle_count;
    const u32 num_thread_groups = (u32)ceilf((f32)num_particles / WARP_SIZE);

    gsk_shader_use(p_particle_system->p_compute_shader);

    // pass uniforms to compute shader
    {
        u32 shader_id = p_particle_system->p_compute_shader->id;

        int num_vert = s_num_vert;
        int rand_idx = rand() % num_vert + 1;

        glUniform1f(glGetUniformLocation(shader_id, "deltaTime"),
                    gsk_device_getTime().delta_time);

        glUniform1f(glGetUniformLocation(shader_id, "curlE"),
                    p_particle_system->noise_cnt);

        glUniform1f(glGetUniformLocation(shader_id, "curlMultiplier"),
                    p_particle_system->noise_multiplier);

        glUniform1f(glGetUniformLocation(shader_id, "particleMinLife"),
                    p_particle_system->min_life);
        glUniform1f(glGetUniformLocation(shader_id, "particleMaxLife"),
                    p_particle_system->max_life);

        glUniform3fv(glGetUniformLocation(shader_id, "emitterPos"),
                     1,
                     (float *)p_particle_system->world_pos);
        glUniform3fv(glGetUniformLocation(shader_id, "emitterScale"),
                     1,
                     (float *)p_particle_system->world_scale);

        glUniform3fv(glGetUniformLocation(shader_id, "emitterRot"),
                     1,
                     (float *)p_particle_system->world_rot);

        glUniform3fv(glGetUniformLocation(shader_id, "convergencePoint"),
                     1,
                     (float *)p_particle_system->convergence_point_world_pos);

        glUniform1f(glGetUniformLocation(shader_id, "convergenceStrength"),
                    p_particle_system->convergence_strength);

        glUniform1f(glGetUniformLocation(shader_id, "totalSmokeDistance"),
                    p_particle_system->ramp_dist);

        glUniform1f(glGetUniformLocation(shader_id, "updraft"),
                    p_particle_system->updraft);

        glUniform1f(glGetUniformLocation(shader_id, "randSeed"), rand_idx);
        glUniform1i(glGetUniformLocation(shader_id, "numVertices"),
                    (float)num_vert);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, p_particle_system->ssbo_mesh_id);
    glBindBufferBase(
      GL_SHADER_STORAGE_BUFFER, 0, p_particle_system->ssbo_mesh_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, p_particle_system->ssbo_particle_id);
    glBindBufferBase(
      GL_SHADER_STORAGE_BUFFER, 1, p_particle_system->ssbo_particle_id);
#if 0
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    0,
                    p_particle_system->mesh_buff_size,
                    p_particle_system->mesh_buff);
#endif

    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_particle_ssbo_id);

    // dispatch to update particles
    glDispatchCompute(num_thread_groups, 1, 1);
}

void
gsk_particle_system_render(gsk_ParticleSystem *p_particle_system)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Default alpha blending
    glBlendEquation(GL_FUNC_ADD);

    // glDisable(GL_DEPTH_TEST);

    gsk_Texture *tex_ramp = GSK_ASSET("gsk://textures/gradient3.png");
    texture_bind(tex_ramp, 9);

    gsk_Texture *tex_main = GSK_ASSET("gsk://textures/particle_fire3.png");
    texture_bind(tex_main, 10);

    u32 shader_id = 0;

    gsk_shader_use(p_particle_system->p_render_shader);
    shader_id = p_particle_system->p_render_shader->id;

#if 0
    glUniform1f(glGetUniformLocation(shader_id, "_SizeByLifeMin"),
                s_size_life_min);
    glUniform1f(glGetUniformLocation(shader_id, "_SizeByLifeMax"),
                s_size_life_max);

    glUniform1f(glGetUniformLocation(shader_id, "_TotalSmokeDistance"),
                s_smoke_dist);
#else
    glUniform1f(glGetUniformLocation(shader_id, "_SizeByLifeMin"),
                p_particle_system->size_life_min);
    glUniform1f(glGetUniformLocation(shader_id, "_SizeByLifeMax"),
                p_particle_system->size_life_max);

    glUniform1f(glGetUniformLocation(shader_id, "_TotalSmokeDistance"),
                p_particle_system->ramp_dist);
#endif

    mat4 newModel = GLM_MAT4_IDENTITY_INIT;
    // glm_translate(newModel, p_particle_system->world_pos);

    glUniformMatrix4fv(glGetUniformLocation(shader_id, "u_Model"),
                       1,
                       GL_FALSE,
                       (float *)newModel);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, p_particle_system->ssbo_particle_id);
    glBindBufferBase(
      GL_SHADER_STORAGE_BUFFER, 1, p_particle_system->ssbo_particle_id);

    glDrawArraysInstanced(GL_POINTS, 0, 1, p_particle_system->particle_count);

    glDisable(GL_BLEND);
    // glEnable(GL_DEPTH_TEST);
}

void
gsk_particle_system_cleanup(gsk_ParticleSystem *p_self)
{
    if (p_self == NULL) { return; }

    if (p_self->particles_buff_size > 0)
    {
        glDeleteBuffers(1, &(p_self->ssbo_particle_id));
        free(p_self->particles_buff);
    }
    if (p_self->mesh_buff > 0)
    {
        glDeleteBuffers(1, &(p_self->ssbo_mesh_id));
        free(p_self->mesh_buff);
    }

    free(p_self);
}