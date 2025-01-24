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
#include "core/drivers/opengl/opengl.h"
#include "core/graphics/shader/shader.h"

#define WARP_SIZE 256.0f

static u32 s_mesh_ubo_id     = 0;
static u32 s_particle_ubo_id = 0;

void
gsk_particle_system_init(gsk_ShaderProgram *p_compute_shader,
                         gsk_ShaderProgram *p_render_shader)
{

    // create UBO's

    s_mesh_ubo_id     = 0;
    s_particle_ubo_id = 0;
    u32 ubo_size      = 0;
    u32 ubo_binding   = 0;

    ubo_size    = sizeof(vec3) * 3;
    ubo_binding = 3;

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Create Uniform Buffer

    glGenBuffers(1, &s_mesh_ubo_id);
    glBindBuffer(GL_UNIFORM_BUFFER, s_mesh_ubo_id);
    glBufferData(GL_UNIFORM_BUFFER, ubo_size, NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(
      GL_UNIFORM_BUFFER, ubo_binding, s_mesh_ubo_id, 0, ubo_size);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Create Uniform Buffer

    ubo_size    = _GSK_PARTICLE_SIZE;
    ubo_binding = 4;

    glGenBuffers(1, &s_particle_ubo_id);
    glBindBuffer(GL_UNIFORM_BUFFER, s_particle_ubo_id);
    glBufferData(GL_UNIFORM_BUFFER, ubo_size, NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(
      GL_UNIFORM_BUFFER, ubo_binding, s_particle_ubo_id, 0, ubo_size);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void
gsk_particle_system_update(gsk_ShaderProgram *p_compute_shader,
                           gsk_ShaderProgram *p_render_shader)
{
    // TODO: update_curl_e();

    const u32 num_particles     = 100;
    const u32 num_thread_groups = (u32)ceilf((f32)num_particles / WARP_SIZE);

    gsk_shader_use(p_compute_shader);

    // pass uniforms to compute shader
    {
        u32 shader_id = p_compute_shader->id;

        GLfloat df = 32.0f;
        vec3 dv    = {0, 0, 0};
        glUniform1f(glGetUniformLocation(shader_id, "deltaTime"), df);
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

        glUniform1f(glGetUniformLocation(shader_id, "totalSmokeDistance"), df);
        glUniform1f(glGetUniformLocation(shader_id, "updraft"), df);
        glUniform1f(glGetUniformLocation(shader_id, "randseed"), df);
        glUniform1i(glGetUniformLocation(shader_id, "numVertices"), df);
#endif
    }

    // dispatch to update particles
    glDispatchCompute(num_thread_groups, 1, 1);
}