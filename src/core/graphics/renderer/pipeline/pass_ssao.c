#include "pass_ssao.h"

#include <core/drivers/opengl/opengl.h>

#include <core/graphics/mesh/primitives.h>
#include <core/graphics/renderer/pipeline/pass_prepass.h>

#include <util/logger.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static ui32 s_ssaoFBO;
static ui32 s_ssaoNoiseTextureId;
static ui32 s_ssaoOutTextureId;

static ShaderProgram *s_ssaoOutShader;
static ShaderProgram *s_ssaoBlurShader;

static VAO *vaoRect;

static float s_ssaoSamples[64];

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
      GL_TEXTURE_2D, 0, GL_RED, 1280, 720, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           s_ssaoOutTextureId,
                           0);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // glBindFramebuffer(GL_FRAMEBUFFER, s_ssaoFBO);
    // glDrawBuffer(GL_NONE);
    // glReadBuffer(GL_NONE);

    // create shader
    s_ssaoOutShader = shader_create_program("../res/shaders/ssao-color.shader");

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer Error %u", status);
    }

    // Sample Kernel
    //--------------

    for (int i = 0; i < 64; i++) {
        vec3 sample = GLM_VEC3_ZERO_INIT;
        sample[0]   = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;
        sample[1]   = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;
        sample[2]   = ((float)rand() / (float)(RAND_MAX)) * 1;
        glm_vec3_normalize(sample);
        glm_vec3_scale(sample, ((float)rand() / (float)(RAND_MAX)) * 1, sample);

        // scale samples so that they cluster near origin
        float scale = (float)i / 64.0f;
        glm_vec3_scale(sample, glm_lerp(0.1f, 1.0f, scale * scale), sample);
        glm_vec3_copy(sample, (vec3 *)s_ssaoSamples + i);
    }
    shader_use(s_ssaoOutShader);
    glUniform3fv(glGetUniformLocation(s_ssaoOutShader->id, "u_samples"),
                 64,
                 s_ssaoSamples);

    // Noise Texture
    //--------------

    float ssaoNoise[16];
    for (int i = 0; i < 16; i++) {
        float x      = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;
        float y      = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;
        ssaoNoise[i] = _noise(x, y);
    }

    glGenTextures(1, &s_ssaoNoiseTextureId);
    glBindTexture(GL_TEXTURE_2D, s_ssaoNoiseTextureId);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create Rectangle
    vaoRect = vao_create();
    vao_bind(vaoRect);
    float *rectPositions = prim_vert_rect();
    VBO *vboRect = vbo_create(rectPositions, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vboRect);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vaoRect, vboRect);
    free(rectPositions);
}

void
pass_ssao_bind()
{
    // Generate SSAO colored output texture from FBO
    //----------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, s_ssaoFBO);
    // Texture inputs
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(s_ssaoOutShader);

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

    // glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    // glDisable(GL_MULTISAMPLE);

    // send samples
    glUniform3fv(glGetUniformLocation(s_ssaoOutShader->id, "u_samples"),
                 64,
                 s_ssaoSamples);

    // draw quad
    vao_bind(vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    // glEnable(GL_DEPTH_TEST);
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