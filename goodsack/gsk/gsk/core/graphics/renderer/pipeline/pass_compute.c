/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "pass_compute.h"

#include "util/sysdefs.h"

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

#include "core/device/device.h"
#include "core/graphics/mesh/primitives.h"

static u32 csTexture;
static gsk_ShaderProgram *csShader;
static gsk_ShaderProgram *shader2;
static gsk_GlVertexArray *vaoRect;

void
computebuffer_init()
{

    // shader Program
    const char *csPath = "../res/shaders/hello.compute";
    csShader           = gsk_shader_compute_program_create(csPath);
    shader2 =
      gsk_shader_program_create("../res/shaders/framebuffer-simple.shader");

    // texture size
    const u32 TEXTURE_WIDTH = 320, TEXTURE_HEIGHT = 180;

    glGenTextures(1, &csTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, csTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA32F,
                 TEXTURE_WIDTH,
                 TEXTURE_HEIGHT,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
                 NULL);

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

static float totalFrames = 0;

void
computebuffer_draw()
{

    totalFrames++;
    glBindImageTexture(0, csTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    gsk_shader_use(csShader);
    glUniform1f(glGetUniformLocation(csShader->id, "t"), totalFrames);
    glDispatchCompute((u32)320, (u32)180, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gsk_shader_use(shader2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, csTexture);
    gsk_gl_vertex_array_bind(vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}
