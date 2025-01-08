/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

/* Final screen pass (Post Processing + MSAA applied) */
#include "pass_screen.h"

#include <stdio.h>
#include <stdlib.h>

#include "util/filesystem.h"
#include "util/gfx.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/mesh/primitives.h"
#include "core/graphics/shader/shader.h"

#include "asset/asset.h"

static u32 msFBO, sbFBO;
static u32 msRBO, sbRBO;
static u32 msTexture, sbTexture;

static gsk_ShaderProgram *shader;
static gsk_GlVertexArray *vaoRect;

static u32 frameWidth, frameHeight;
static u32 ms_samples = 4;

static void
CreateMultisampleBuffer(u32 samples, u32 width, u32 height)
{
    ms_samples = samples;

    // Create texture
    glGenTextures(1, &msTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                            ms_samples,
                            GL_RGBA16F,
                            width,
                            height,
                            GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Create Framebuffer object
    glGenFramebuffers(1, &msFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D_MULTISAMPLE,
                           msTexture,
                           0);

    // Create Renderbuffer object
    glGenRenderbuffers(1, &msRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, msRBO);
    glRenderbufferStorageMultisample(
      GL_RENDERBUFFER, ms_samples, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Attach Renderbuffer
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msRBO);

    // Error checking
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_CRITICAL("\nFramebuffer failed: %u\n", status);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void
CreateScreenBuffer(u32 width, u32 height)
{
    // Create Texture
    glGenTextures(1, &sbTexture);
    glBindTexture(GL_TEXTURE_2D, sbTexture);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    // Create Framebuffer object
    glGenFramebuffers(1, &sbFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sbFBO);
    // Attach texture to FBO
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sbTexture, 0);

    // Create Renderbuffer object [Depth]
    glGenRenderbuffers(1, &sbRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, sbRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Attach Renderbuffer
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sbRBO);

    // Error checking
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_CRITICAL("\nFramebuffer failed: %u\n", status);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
postbuffer_resize(u32 winWidth, u32 winHeight)
{
    LOG_INFO("Resizing window to %d x %d", winWidth, winHeight);

    GLenum err = NULL;

    glBindFramebuffer(GL_FRAMEBUFFER, sbFBO);

    glBindTexture(GL_TEXTURE_2D, sbTexture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA16F,
                 winWidth,
                 winHeight,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
                 NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, sbRBO);
    glRenderbufferStorage(
      GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, winWidth, winHeight);

    err = glGetError();
    if (err != GL_NO_ERROR) { LOG_CRITICAL("glTexImage2D error: 0x%x", err); }

    glBindFramebuffer(GL_FRAMEBUFFER, msFBO);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                            ms_samples,
                            GL_RGBA16F,
                            winWidth,
                            winHeight,
                            GL_TRUE);

    glBindRenderbuffer(GL_RENDERBUFFER, msRBO);
    glRenderbufferStorageMultisample(
      GL_RENDERBUFFER, ms_samples, GL_DEPTH24_STENCIL8, winWidth, winHeight);

    err = glGetError();
    if (err != GL_NO_ERROR)
    {
        LOG_CRITICAL("glTexImage2DMultisample error: 0x%x", err);
    }

    frameWidth  = winWidth;
    frameHeight = winHeight;
}

void
postbuffer_init(u32 width, u32 height, gsk_RendererProps *properties)
{
    frameWidth  = width;
    frameHeight = height;

    // Shader
    shader = GSK_ASSET("gsk://shaders/framebuffer.shader");
    gsk_shader_use(shader);
    glUniform1i(glGetUniformLocation(shader->id, "u_ScreenTexture"), 0);
    glUniform1i(glGetUniformLocation(shader->id, "u_BloomTexture"), 1);

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

    // Create Framebuffer
    CreateScreenBuffer(width, height);
    // Create MSAA buffer
    CreateMultisampleBuffer(properties->msaaSamples, width, height);
}

void
postbuffer_bind(int enableMSAA)
{
    // glDebugMessageInsert(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_MARKER, 0,
    //     GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Post Processing buffer init");
    if (enableMSAA)
    {
        glEnable(GL_MULTISAMPLE);
        glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
    } else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, sbFBO);
    }

    // Prime
    glViewport(0, 0, frameWidth, frameHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec4 col = DEFAULT_CLEAR_COLOR;
    glClearColor(col[0], col[1], col[2], col[3]);
}

void
postbuffer_draw(gsk_RendererProps *properties, u32 bloom_texture_id)
{

    if (properties->msaaEnable)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sbFBO);
        glBlitFramebuffer(0,
                          0,
                          frameWidth,
                          frameHeight,
                          0,
                          0,
                          frameWidth,
                          frameHeight,
                          GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TODO: Set to screen resolution
    glViewport(0, 0, frameWidth, frameHeight);

    gsk_gl_vertex_array_bind(vaoRect);
    gsk_shader_use(shader);
    glUniform1i(glGetUniformLocation(shader->id, "u_Tonemapper"),
                properties->tonemapper);
    glUniform1f(glGetUniformLocation(shader->id, "u_Exposure"),
                properties->exposure);
    glUniform1f(glGetUniformLocation(shader->id, "u_MaxWhite"),
                properties->maxWhite);
    glUniform1f(glGetUniformLocation(shader->id, "u_Gamma"), properties->gamma);
    glUniform1i(glGetUniformLocation(shader->id, "u_GammaEnable"),
                properties->gammaEnable);

    glUniform1f(glGetUniformLocation(shader->id, "u_VignetteAmount"),
                properties->vignetteAmount);
    glUniform1f(glGetUniformLocation(shader->id, "u_VignetteFalloff"),
                properties->vignetteFalloff);
    glUniform3fv(glGetUniformLocation(shader->id, "u_VignetteColor"),
                 1,
                 (float *)properties->vignetteColor);

    glUniform1f(glGetUniformLocation(shader->id, "u_BloomIntensity"),
                properties->bloom_intensity);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sbTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom_texture_id);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

#if 0
    // second post processing effect (Read from MAIN FBO ???)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sbFBO);
    glBlitFramebuffer(0,
                      0,
                      frameWidth,
                      frameHeight,
                      0,
                      0,
                      frameWidth,
                      frameHeight,
                      GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    gsk_gl_vertex_array_bind(vaoRect);
    gsk_shader_use(shader2);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sbTexture);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
#endif
}

void
postbuffer_cleanup()
{
    glDeleteProgram(shader->id);
}

u32
postbuffer_get_id()
{
    // TODO: Check if MSAA enabled
    return sbTexture;
}
