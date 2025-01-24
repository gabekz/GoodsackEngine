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
        sp_particles[i].life =
          ((double)rand() / (double)RAND_MAX) * 5.0f + 1.0f;
    }

    // create SSBO's

    s_mesh_ssbo_id     = 0;
    s_particle_ssbo_id = 0;
    u32 ssbo_size      = 0;
    u32 ssbo_binding   = 0;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Create mesh SSBO

    ssbo_size    = sizeof(vec3) * 3;
    ssbo_binding = 0;

    glGenBuffers(1, &s_mesh_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_mesh_ssbo_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_size, NULL, GL_DYNAMIC_DRAW);
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
    // TODO: update_curl_e();

    const u32 num_particles     = _GSK_PARTICLE_COUNT;
    const u32 num_thread_groups = (u32)ceilf((f32)num_particles / WARP_SIZE);

    gsk_shader_use(p_compute_shader);

    // pass uniforms to compute shader
    {
        u32 shader_id = p_compute_shader->id;

        GLfloat df = 32.0f;
        vec3 dv    = {0, 0, 0};

        glUniform1f(glGetUniformLocation(shader_id, "deltaTime"),
                    gsk_device_getTime().delta_time);
#if 1
        glUniform1f(glGetUniformLocation(shader_id, "curlE"), df);
        glUniform1f(glGetUniformLocation(shader_id, "curlMultiplier"), df);
        glUniform1f(glGetUniformLocation(shader_id, "particleMinLife"), df);
        glUniform1f(glGetUniformLocation(shader_id, "particleMaxLife"), df);

        glUniform3fv(
          glGetUniformLocation(shader_id, "emitterPos"), 1, (float *)dv);
        glUniform3fv(
          glGetUniformLocation(shader_id, "emitterScale"), 1, (float *)dv);
        glUniform3fv(
          glGetUniformLocation(shader_id, "emitterRot"), 1, (float *)dv);
        glUniform3fv(
          glGetUniformLocation(shader_id, "convergencePoint"), 1, (float *)dv);

        glUniform1f(glGetUniformLocation(shader_id, "convergenceStrength"), df);

        glUniform1f(glGetUniformLocation(shader_id, "totalSmokeDistance"), df);
        glUniform1f(glGetUniformLocation(shader_id, "updraft"), df);
        glUniform1f(glGetUniformLocation(shader_id, "randseed"), df);
        glUniform1i(glGetUniformLocation(shader_id, "numVertices"), df);
#endif
    }

    // dispatch to update particles
    glDispatchCompute(num_thread_groups, 1, 1);
}

void
gsk_particle_system_render(gsk_ShaderProgram *p_render_shader)
{
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

    GLfloat df = 32.0f;

    glUniform1f(glGetUniformLocation(shader_id, "_SizeByLifeMin"), df);
    glUniform1f(glGetUniformLocation(shader_id, "_SizeByLifeMax"), df);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_mesh_ssbo_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_particle_ssbo_id);

    glDrawArraysInstanced(GL_POINTS, 0, 1, _GSK_PARTICLE_COUNT);
}