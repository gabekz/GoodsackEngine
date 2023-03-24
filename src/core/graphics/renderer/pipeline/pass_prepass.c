#include "pass_prepass.h"

#include <core/graphics/material/material.h>
#include <core/graphics/mesh/primitives.h>
#include <core/graphics/shader/shader.h>

#include <core/drivers/opengl/opengl.h>

static ShaderProgram *s_depthPrepassShader;
static Material *s_depthPrepassMaterial;

static ui32 s_depthPrepassFBO;
static ui32 s_depthPrepassTextureId;

// GBuffer information
static ui32 s_gPosition;
static ui32 s_gNormal;

static VAO *s_vaoRect;

void
prepass_init()
{
    // Create the depthmap (screenspace) FBO
    glGenFramebuffers(1, &s_depthPrepassFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_depthPrepassFBO);

    // - position color buffer TODO: width/height
    glGenTextures(1, &s_gPosition);
    glBindTexture(GL_TEXTURE_2D, s_gPosition);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_gPosition, 0);

    // - normal color buffer TODO: width/height
    glGenTextures(1, &s_gNormal);
    glBindTexture(GL_TEXTURE_2D, s_gNormal);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, 1280, 720, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, s_gNormal, 0);

    // Renderbuffer
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Texture
    /*
    glGenTextures(1, &s_depthPrepassTextureId);
    glBindTexture(GL_TEXTURE_2D, s_depthPrepassTextureId);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 1280, // TODO: WIDTH
                 720,  // TODO: HEIGHT
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    */

    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

    /*
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           // GL_DEPTH_ATTACHMENT,
                           attachments,
                           GL_TEXTURE_2D,
                           s_depthPrepassTextureId,
                           0);
                           */
    glDrawBuffers(2, attachments);

    // create shader and material
    s_depthPrepassShader =
      shader_create_program("../res/shaders/depth-prepass.shader");
    s_depthPrepassMaterial = material_create(s_depthPrepassShader, NULL, 0);

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

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer Error %u", status);
    }

    // unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
prepass_bindTextures(ui32 startingSlot)
{
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0 + startingSlot);
    glBindTexture(GL_TEXTURE_2D, s_gPosition);
    glActiveTexture(GL_TEXTURE1 + startingSlot);
    glBindTexture(GL_TEXTURE_2D, s_gNormal);
}

void
prepass_bind()
{

    glBindFramebuffer(GL_FRAMEBUFFER, s_depthPrepassFBO);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 1280, 720); // TODO: width and height

    shader_use(s_depthPrepassShader);

    // Bind the shadowmap to texture slot 0
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, s_depthPrepassTextureId);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepass_bindTextures(0);
}

void
prepass_draw()
{
    vao_bind(s_vaoRect);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Material *
prepass_getMaterial()
{
    return s_depthPrepassMaterial;
}

ui32
prepass_getPosition()
{
    return s_gPosition;
}
ui32
prepass_getNormal()
{
    return s_gNormal;
}
