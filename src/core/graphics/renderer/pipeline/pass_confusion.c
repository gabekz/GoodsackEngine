#include "pass_confusion.h"

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/shader/shader.h>
#include <util/sysdefs.h>

#include <core/graphics/renderer/pipeline/pass_prepass.h>
#include <core/graphics/renderer/pipeline/pass_screen.h>

#include <core/graphics/mesh/primitives.h>

static ui32 s_confusionFBO;
static ui32 s_confusionTextureId;  // dNear texture
static ui32 s_confusionTextureId2; // dFar texture

static ui32 s_confusionBlurFBO;
static ui32 s_confusionBlurTextureId;

static ShaderProgram *s_confusionShader;
static ShaderProgram *s_confusionBlurShader;

static VAO *s_vaoRect;

void
pass_confusion_init()
{
    glGenFramebuffers(1, &s_confusionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_confusionFBO);

    // Texture
    glGenTextures(1, &s_confusionTextureId);
    glBindTexture(GL_TEXTURE_2D, s_confusionTextureId);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           s_confusionTextureId,
                           0);
    // Texture 2
    glGenTextures(1, &s_confusionTextureId2);
    glBindTexture(GL_TEXTURE_2D, s_confusionTextureId2);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D,
                           s_confusionTextureId2,
                           0);

    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    s_confusionShader =
      shader_create_program("../res/shaders/confusion.shader");

    s_confusionBlurShader =
      shader_create_program("../res/shaders/confusion-blur.shader");

    // Check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer Error %u", status);
    }

    glGenFramebuffers(1, &s_confusionBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_confusionBlurFBO);

    // Texture
    glGenTextures(1, &s_confusionBlurTextureId);
    glBindTexture(GL_TEXTURE_2D, s_confusionBlurTextureId);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           s_confusionTextureId,
                           0);
    // Check FBO status
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer Error %u", status);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create Rectangle
    s_vaoRect = vao_create();
    vao_bind(s_vaoRect);
    float *rectPositions = prim_vert_rect();
    VBO *vboRect = vbo_create(rectPositions, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vboRect);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(s_vaoRect, vboRect);
    free(rectPositions);
}

void
pass_confusion_draw()
{
    glBindFramebuffer(GL_FRAMEBUFFER, s_confusionFBO);

    shader_use(s_confusionShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, prepass_getPosition());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, postbuffer_getScreenTexture());

    vao_bind(s_vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    // Now do some blur

    glBindFramebuffer(GL_FRAMEBUFFER, s_confusionBlurFBO);
    shader_use(s_confusionBlurShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_confusionTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, s_confusionTextureId2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}