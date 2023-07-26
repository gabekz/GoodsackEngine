/* Final screen pass (Post Processing + MSAA applied) */
#include "pass_screen.h"

#include <stdio.h>
#include <stdlib.h>

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/mesh/primitives.h>
#include <core/graphics/shader/shader.h>

#include <util/gfx.h>
#include <util/logger.h>
#include <util/sysdefs.h>

static ui32 msFBO, sbFBO;
static ui32 msRBO, sbRBO;
static ui32 msTexture, sbTexture;

static ShaderProgram *shader;
static VAO *vaoRect;

static ui32 frameWidth, frameHeight;

static void
CreateMultisampleBuffer(ui32 samples, ui32 width, ui32 height)
{
    // Create texture
    glGenTextures(1, &msTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture);
    glTexImage2DMultisample(
      GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA16F, width, height, GL_TRUE);
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
      GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Attach Renderbuffer
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void
CreateScreenBuffer(ui32 width, ui32 height)
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
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("\nFramebuffer ERROR: %u\n", status);
    }
}

void
postbuffer_resize(ui32 winWidth, ui32 winHeight)
{
    LOG_INFO("Resizing window to %d x %d", winWidth, winHeight);

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

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture);
    glTexImage2DMultisample(
      GL_TEXTURE_2D_MULTISAMPLE, 16, GL_RGBA16F, winWidth, winHeight, GL_TRUE);

    glBindRenderbuffer(GL_RENDERBUFFER, msRBO);
    glRenderbufferStorageMultisample(
      GL_RENDERBUFFER, 16, GL_DEPTH24_STENCIL8, winWidth, winHeight);

    frameWidth  = winWidth;
    frameHeight = winHeight;
}

void
postbuffer_init(ui32 width, ui32 height, RendererProps *properties)
{
    frameWidth  = width;
    frameHeight = height;

    // Shader
    shader = shader_create_program("../res/shaders/framebuffer.shader");
    shader_use(shader);
    glUniform1i(glGetUniformLocation(shader->id, "u_ScreenTexture"), 0);

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
    if (enableMSAA) {
        glEnable(GL_MULTISAMPLE);
        glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, sbFBO);
    }

    // Prime
    glViewport(0, 0, frameWidth, frameHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec4 col = DEFAULT_CLEAR_COLOR;
    glClearColor(col[0], col[1], col[2], col[3]);
}

void
postbuffer_draw(RendererProps *properties)
{

    if (properties->msaaEnable) {
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
    glViewport(0, 0, frameWidth, frameHeight);

    vao_bind(vaoRect);
    shader_use(shader);
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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sbTexture);
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

    vao_bind(vaoRect);
    shader_use(shader2);
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
