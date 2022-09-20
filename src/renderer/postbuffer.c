/* postbuffer.c ~ singleton framebuffer program */

#include "postbuffer.h"
#include "../glbuffer/glbuffer.h"
#include "../gfx.h"
#include "../shader.h"
#include <model/primitives.h>

#include <util/sysdefs.h>

#include <stdlib.h>
#include <stdio.h>

static ui32 msFBO, sbFBO;
static ui32 msRBO, sbRBO;
static ui32 msTexture, sbTexture;

static ShaderProgram *shader;
static VAO *vaoRect;

static void CreateMultisampleBuffer(ui32 samples, ui32 width, ui32 height) {
// Create texture
    glGenTextures(1, &msTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB,
        width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

// Create Framebuffer object
    glGenFramebuffers(1, &msFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, msFBO);
    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D_MULTISAMPLE, msTexture, 0);

// Create Renderbuffer object
    glGenRenderbuffers(1, &msRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, msRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4,
        GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Attach Renderbuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, msRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void CreateScreenBuffer(ui32 width, ui32 height) {
// Create Texture
    glGenTextures(1, &sbTexture);
    glBindTexture(GL_TEXTURE_2D, sbTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
        0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

// Create Framebuffer object
    glGenFramebuffers(1, &sbFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sbFBO);
    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, sbTexture, 0);

// Create Renderbuffer object [Depth]
    glGenRenderbuffers(1, &sbRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, sbRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
        width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Attach Renderbuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, sbRBO);

    // Error checking
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        printf("\nFramebuffer ERROR: %u\n", status);
    }
}

void postbuffer_init(ui32 winWidth, ui32 winHeight) {
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
    CreateScreenBuffer(winWidth, winHeight);
}

void postbuffer_bind() {
    //glDebugMessageInsert(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_MARKER, 0,                       
    //    GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Post Processing buffer init");
    glBindFramebuffer(GL_FRAMEBUFFER, sbFBO);
}

void postbuffer_draw() {
        vao_bind(vaoRect);
        shader_use(shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sbTexture);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}

void postbuffer_cleanup() {
    glDeleteProgram(shader->id);
}
