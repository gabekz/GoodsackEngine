/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "pass_ssao.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/mesh/primitives.h"
#include "core/graphics/renderer/pipeline/pass_prepass.h"

static vec3 s_ssaoSamples[64];
static u32 s_ssaoFBO;
static u32 s_ssaoNoiseTextureId;
static u32 s_ssaoOutTextureId;
static gsk_ShaderProgram *s_ssaoOutShader;

static u32 s_ssaoBlurFBO;
static u32 s_ssaoBlurOutTextureId;
static gsk_ShaderProgram *s_ssaoBlurShader;

static gsk_GlVertexArray *vaoRect;

static float
_noise(int x, int y)
{
    int n;

    n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0 - ((n * ((n * n * 15731) + 789221) + 1376312589) & 0x7fffffff) /
                    1073741824.0);
}
void
pass_ssao_init()
{
    // SSAO Framebuffer
    // ---------------

    glGenFramebuffers(1, &s_ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_ssaoFBO);
    // Texture
    glGenTextures(1, &s_ssaoOutTextureId);
    glBindTexture(GL_TEXTURE_2D, s_ssaoOutTextureId);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RED, 1280, 720, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           s_ssaoOutTextureId,
                           0);

    // glBindFramebuffer(GL_FRAMEBUFFER, s_ssaoFBO);
    // glDrawBuffer(GL_NONE);
    // glReadBuffer(GL_NONE);

    // Check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERROR("Framebuffer Error %u", status);
    }
    // reset FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SSAO Blur
    // ---------------
    glGenFramebuffers(1, &s_ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_ssaoBlurFBO);
    // Texture
    glGenTextures(1, &s_ssaoBlurOutTextureId);
    glBindTexture(GL_TEXTURE_2D, s_ssaoBlurOutTextureId);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RED, 1280, 720, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           s_ssaoBlurOutTextureId,
                           0);

    // Check FBO status
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERROR("Framebuffer Error %u", status);
    }
    // reset FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create Shaders
    //---------------
    s_ssaoOutShader =
      gsk_shader_program_create(GSK_PATH("gsk://shaders/ssao-color.shader"));
    s_ssaoBlurShader =
      gsk_shader_program_create(GSK_PATH("gsk://shaders/ssao-blur.shader"));

    // Sample Kernel
    //--------------

    for (int i = 0; i < 64; i++)
    {
        vec3 sample = GLM_VEC3_ZERO_INIT;
        sample[0]   = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;
        sample[1]   = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;
        sample[2]   = ((float)rand() / (float)(RAND_MAX)) * 1;
        glm_vec3_normalize(sample);
        glm_vec3_scale(sample, ((float)rand() / (float)(RAND_MAX)) * 1, sample);

        // scale samples so that they cluster near origin
        float scale = (float)i / 64.0f;
        glm_vec3_scale(sample, glm_lerp(0.1f, 1.0f, scale * scale), sample);
        glm_vec3_copy(sample, s_ssaoSamples[i]);
    }
    /*
    gsk_shader_use(s_ssaoOutShader);
    glUniform3fv(glGetUniformLocation(s_ssaoOutShader->id, "u_samples"),
                 64,
                 s_ssaoSamples);
                 */

    // Noise Texture
    //--------------

    vec3 ssaoNoise[16];
    for (int i = 0; i < 16; i++)
    {
        float x = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;
        float y = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;

        vec3 noisePixel = GLM_VEC3_ZERO_INIT;
        noisePixel[0]   = _noise(x, y + i);
        noisePixel[1]   = _noise(x + i, y + i);
        noisePixel[2]   = 0.0f;

        glm_vec3_copy(noisePixel, ssaoNoise[i]);
    }

    glGenTextures(1, &s_ssaoNoiseTextureId);
    glBindTexture(GL_TEXTURE_2D, s_ssaoNoiseTextureId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA16F,
                 4,
                 4,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 (float *)ssaoNoise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Create Rectangle
    vaoRect = gsk_gl_vertex_array_create();
    gsk_gl_vertex_array_bind(vaoRect);
    float *rectPositions = prim_vert_rect();
    gsk_GlVertexBuffer *vboRect =
      gsk_gl_vertex_buffer_create(rectPositions, (2 * 3 * 4) * sizeof(float));
    gsk_gl_vertex_buffer_bind(vboRect);
    gsk_gl_vertex_buffer_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_buffer_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_array_add_buffer(vaoRect, vboRect);
    free(rectPositions);
}

void
pass_ssao_bind(SsaoOptions options)
{
    // Generate SSAO colored output texture from FBO
    //----------------------------------------------

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, s_ssaoFBO);

    glViewport(0, 0, 1280, 720);

    // Texture inputs
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gsk_shader_use(s_ssaoOutShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_ssaoOutTextureId);

    prepass_bindTextures(1); // bind gPosition and gNormal
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, s_ssaoNoiseTextureId);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           s_ssaoOutTextureId,
                           0);
    // send samples
    glUniform3fv(glGetUniformLocation(s_ssaoOutShader->id, "u_samples"),
                 64,
                 (float *)s_ssaoSamples);

    // radius
    glUniform1f(glGetUniformLocation(s_ssaoOutShader->id, "u_radius"),
                options.radius);
    // bias
    glUniform1f(glGetUniformLocation(s_ssaoOutShader->id, "u_bias"),
                options.bias);
    // kernel size
    glUniform1i(glGetUniformLocation(s_ssaoOutShader->id, "u_kernelSize"),
                options.kernelSize);

    // draw quad
    gsk_gl_vertex_array_bind(vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    // blur SSAO
    glBindFramebuffer(GL_FRAMEBUFFER, s_ssaoBlurFBO);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gsk_shader_use(s_ssaoBlurShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_ssaoBlurOutTextureId);

    // Attach SSAO output
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, s_ssaoOutTextureId);

    // draw quad
    gsk_gl_vertex_array_bind(vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // reset

    /*
    // Blur SSAO Texture
    //----------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, s_ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    // shader use BLUR
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_ssaoOutTextureId);
    // render quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // reset
    */
}

u32
pass_ssao_getOutputTextureId()
{
    return s_ssaoBlurOutTextureId;
}