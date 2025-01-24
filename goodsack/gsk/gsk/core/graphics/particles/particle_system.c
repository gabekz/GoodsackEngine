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

#define WARP_SIZE 256.0f

static u32 s_mesh_ssbo_id     = 0;
static u32 s_particle_ssbo_id = 0;

static gsk_ShaderProgram *s_saved_render_shader;

static gsk_Particle sp_particles[_GSK_PARTICLE_COUNT];

static f32 s_curl_E_min        = 0.1f;
static f32 s_curl_E_max        = 0.3f;
static f32 s_curl_E_multiplier = 0.05f;
static f32 s_curl_E_speed      = 1.0f;
static f32 s_curl_E            = 0;

static f32 s_min_life = 2.0f;
static f32 s_max_life = 6.0f;

void
_update_curl()
{
    s_curl_E = glm_lerp(
      s_curl_E_min,
      s_curl_E_max,
      sin(gsk_device_getTime().time_elapsed * s_curl_E_speed) / 2.0f + 0.5f);
}

void
gsk_particle_system_init(gsk_ShaderProgram *p_compute_shader,
                         gsk_ShaderProgram *p_render_shader)
{

    s_saved_render_shader = p_render_shader;

    // initialize the particles
    const u32 particle_count = _GSK_PARTICLE_COUNT;

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

    s_mesh_ssbo_id     = 0;
    s_particle_ssbo_id = 0;
    u32 ssbo_size      = 0;
    u32 ssbo_binding   = 0;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Create mesh SSBO

    vec4 triangle[6] = {{0, 0, 0, 8},
                        {0, 0, 1, 9},
                        {0, 1, 0, 10},
                        {0, 0, 0, 8},
                        {0, 0, 1, 9},
                        {0, 1, 0, 10}};

    ssbo_size    = sizeof(vec4) * 3;
    ssbo_binding = 0;

    glGenBuffers(1, &s_mesh_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_mesh_ssbo_id);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER, ssbo_size * 2, triangle, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding, s_mesh_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Create particle SSBO

    ssbo_size    = _GSK_PARTICLE_SIZE;
    ssbo_binding = 1;

    glGenBuffers(1, &s_particle_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_particle_ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 ssbo_size * _GSK_PARTICLE_COUNT,
                 sp_particles,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(
      GL_SHADER_STORAGE_BUFFER, ssbo_binding, s_particle_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void
gsk_particle_system_update(gsk_ShaderProgram *p_compute_shader,
                           gsk_ShaderProgram *p_render_shader)
{

    _update_curl();

    const u32 num_particles     = _GSK_PARTICLE_COUNT;
    const u32 num_thread_groups = (u32)ceilf((f32)num_particles / WARP_SIZE);

    gsk_shader_use(p_compute_shader);

    // pass uniforms to compute shader
    {
        u32 shader_id = p_compute_shader->id;

        GLfloat df  = 1.0f;
        GLfloat df0 = 0.0f;
        GLfloat df1 = 0.025f;
        GLfloat df3 = 0.3f;
        vec3 dv     = {1, 1, 0};
        vec3 dv0    = {0, 0, 0};

        f32 max_life = 5;
        f32 num_vert = 6;

        glUniform1f(glGetUniformLocation(shader_id, "deltaTime"),
                    gsk_device_getTime().delta_time);
#if 1
        glUniform1f(glGetUniformLocation(shader_id, "curlE"), s_curl_E);
        glUniform1f(glGetUniformLocation(shader_id, "curlMultiplier"),
                    s_curl_E_multiplier);
        glUniform1f(glGetUniformLocation(shader_id, "particleMinLife"), df);
        glUniform1f(glGetUniformLocation(shader_id, "particleMaxLife"),
                    max_life);

        glUniform3fv(
          glGetUniformLocation(shader_id, "emitterPos"), 1, (float *)dv0);
        glUniform3fv(
          glGetUniformLocation(shader_id, "emitterScale"), 1, (float *)dv);
        glUniform3fv(
          glGetUniformLocation(shader_id, "emitterRot"), 1, (float *)dv0);
        glUniform3fv(
          glGetUniformLocation(shader_id, "convergencePoint"), 1, (float *)dv);

        glUniform1f(glGetUniformLocation(shader_id, "convergenceStrength"), df);

        glUniform1f(glGetUniformLocation(shader_id, "totalSmokeDistance"), df3);
        glUniform1f(glGetUniformLocation(shader_id, "updraft"), df1);
        glUniform1f(glGetUniformLocation(shader_id, "randseed"), df);
        glUniform1i(glGetUniformLocation(shader_id, "numVertices"), num_vert);
#endif
    }

    // dispatch to update particles
    glDispatchCompute(num_thread_groups, 1, 1);
}

void
gsk_particle_system_render(gsk_ShaderProgram *p_render_shader)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Default alpha blending
    glBlendEquation(GL_FUNC_ADD);

    u32 shader_id = 0;

    if (p_render_shader == NULL)
    {

        gsk_shader_use(s_saved_render_shader);
        shader_id = s_saved_render_shader->id;
    } else
    {
        gsk_shader_use(p_render_shader);
        shader_id = p_render_shader->id;
    }

    GLfloat df01 = 0.1f;
    GLfloat df   = 0.3f;

    glUniform1f(glGetUniformLocation(shader_id, "_SizeByLifeMin"), df01);
    glUniform1f(glGetUniformLocation(shader_id, "_SizeByLifeMax"), df);

    glUniform1f(glGetUniformLocation(shader_id, "_TotalSmokeDistance"), df);

    mat4 newModel = GLM_MAT4_IDENTITY_INIT;
    glUniformMatrix4fv(glGetUniformLocation(shader_id, "u_Model"),
                       1,
                       GL_FALSE,
                       (float *)newModel);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_mesh_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_particle_ssbo_id);

    glDrawArraysInstanced(GL_POINTS, 0, 1, _GSK_PARTICLE_COUNT);

    glDisable(GL_BLEND);
}