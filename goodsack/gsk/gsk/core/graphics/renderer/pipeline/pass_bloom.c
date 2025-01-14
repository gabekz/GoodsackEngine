/*
 * Copyright (c) 2025-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */
#include "pass_bloom.h"

#include "asset/asset.h"
#include "core/drivers/opengl/opengl.h"
#include "core/graphics/material/material.h"
#include "core/graphics/mesh/primitives.h"
#include "core/graphics/shader/shader.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#define MIP_LENGTH 6

typedef struct gsk_BloomMip
{
    vec2 size;
    u32 texture;
} gsk_BloomMip;

static u32 bloom_fbo;
static gsk_ShaderProgram *s_shader_downsample;
static gsk_ShaderProgram *s_shader_upsample;

static gsk_GlVertexArray *vaoRect;

static gsk_BloomMip s_p_mips[MIP_LENGTH];

static void
_render_downsamples(u32 tex_source_id, f32 threshold);
static void
_render_upsamples(f32 filter_radius);

void
pass_bloom_init()
{
    u32 mip_chain_length = MIP_LENGTH;
    // gsk_BloomMip *p_mips = malloc(sizeof(gsk_BloomMip) * mip_chain_length);

    glGenFramebuffers(1, &bloom_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);

    vec2 mip_size = {1280, 720};

    for (int i = 0; i < mip_chain_length; i++)
    {
        gsk_BloomMip mip = {0};

        mip_size[0] *= 0.5f;
        mip_size[1] *= 0.5f;

        glm_vec2_copy(mip_size, mip.size);

        mip.size[0] = (int)mip.size[0];
        mip.size[1] = (int)mip.size[1];

        glGenTextures(1, &mip.texture);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        // we are downscaling an HDR color buffer, so we need a float texture
        // format
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_R11F_G11F_B10F,
                     (int)mip_size[0],
                     (int)mip_size[1],
                     0,
                     GL_RGB,
                     GL_FLOAT,
                     NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        s_p_mips[i] = mip;
    }

    // setup attachments

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           s_p_mips[0].texture,
                           0);

    u32 attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    // check completion status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERROR("FBO error, status: 0x\%x\n", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // load shaders
    s_shader_downsample = GSK_ASSET("gsk://shaders/bloom/downsample.shader");
    s_shader_upsample   = GSK_ASSET("gsk://shaders/bloom/upsample.shader");

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

    // Cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
pass_bloom_render(u32 tex_source_id, gsk_RendererProps *properties)
{

    glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);

    _render_downsamples(tex_source_id, properties->bloom_threshold);
    _render_upsamples(properties->bloom_radius);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1280, 720);
}

static void
_render_downsamples(u32 tex_source_id, f32 threshold)
{
    glDisable(GL_BLEND);

    vec2 viewport = {1280, 720};

    // shader
    gsk_shader_use(s_shader_downsample);

    glUniform2fv(
      glGetUniformLocation(s_shader_downsample->id, "u_src_resolution"),
      1,
      (float *)viewport); // TODO: CHANGE TO SOURCE VIEWPORT RESOLUTION

    // prefilter for first pass
    glUniform1i(glGetUniformLocation(s_shader_downsample->id, "u_DoPrefilter"),
                TRUE);

    // send threshold
    glUniform1f(glGetUniformLocation(s_shader_downsample->id, "u_Threshold"),
                threshold);

    // HDR color texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_source_id);

    // downsample through mip chain
    for (int i = 0; i < MIP_LENGTH; i++)
    {
        // get mip from index HERE
        gsk_BloomMip mip = s_p_mips[i];

        glViewport(0, 0, mip.size[0], mip.size[1]);
        glFramebufferTexture2D(
          GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

        // draw quad
        gsk_gl_vertex_array_bind(vaoRect);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

        // set new source resolution HERE to mip.size
        glUniform2fv(
          glGetUniformLocation(s_shader_downsample->id, "u_src_resolution"),
          1,
          (float *)mip.size);

        // bind new texture
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        if (i == 0)
        {
            // disable prefilter after first iteration
            glUniform1ui(
              glGetUniformLocation(s_shader_downsample->id, "u_DoPrefilter"),
              FALSE);
        }
    }
    glEnable(GL_BLEND);
}

static void
_render_upsamples(f32 filter_radius)
{
    vec2 viewport    = {1280, 720};
    f32 aspect_ratio = viewport[0] / viewport[1];

    // shaders
    gsk_shader_use(s_shader_upsample);
    glUniform1f(glGetUniformLocation(s_shader_upsample->id, "u_filter_radius"),
                filter_radius);

    // send aspect ratio
    glUniform1f(glGetUniformLocation(s_shader_downsample->id, "u_AspectRatio"),
                aspect_ratio);

    // Enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Default alpha blending
    // glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    // downsample through mip chain
    for (int i = MIP_LENGTH - 1; i > 0; i--)
    {
        gsk_BloomMip mip      = s_p_mips[i];
        gsk_BloomMip next_mip = s_p_mips[i - 1];

        // Bind viewport and texture from where to read
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        // Set framebuffer render target (we write to this texture)
        glViewport(0, 0, next_mip.size[0], next_mip.size[1]);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               next_mip.texture,
                               0);

        // draw quad
        gsk_gl_vertex_array_bind(vaoRect);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    }

    // Disable additive blending
    glDisable(GL_BLEND);
}

u32
pass_bloom_get_texture_id()
{
    return s_p_mips[0].texture;
}