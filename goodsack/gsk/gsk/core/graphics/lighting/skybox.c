/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "skybox.h"

#include "util/filesystem.h"

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/primitives.h"
#include "core/graphics/shader/shader.h"

#include "asset/asset.h"
#include "runtime/gsk_runtime_wrapper.h"

static u32 cubemapProjectionFBO;
static u32 cubemapProjectionRBO;
static gsk_GlVertexArray *cubemapProjectionVAO;
static gsk_ShaderProgram *cubemapProjectionShader;
static gsk_ShaderProgram *cubemapShaderConvolute;
static gsk_ShaderProgram *cubemapShaderPrefilter;
static gsk_ShaderProgram *cubemapBrdfShader;

gsk_Skybox *
gsk_skybox_create(gsk_Texture *cubemap)
{
    gsk_Skybox *ret = malloc(sizeof(gsk_Skybox));
    ret->cubemap    = cubemap;

    gsk_GlVertexArray *vao = gsk_gl_vertex_array_create();
    ret->vao               = vao;

    gsk_GlVertexBuffer *vbo = gsk_gl_vertex_buffer_create(
      PRIM_ARR_V_CUBE, PRIM_SIZ_V_CUBE * sizeof(float));
    gsk_gl_vertex_buffer_push(vbo, 3, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_array_add_buffer(vao, vbo);
    gsk_GlIndexBuffer *ibo = gsk_gl_index_buffer_create(
      PRIM_ARR_I_CUBE, PRIM_SIZ_I_CUBE * sizeof(unsigned int));
    gsk_gl_index_buffer_bind(ibo);
    free(vbo);
    gsk_ShaderProgram *shader = GSK_ASSET("gsk://shaders/skybox.shader");
    ret->shader               = shader;

    return ret;
}

void
gsk_skybox_draw(gsk_Skybox *self)
{
    // glDepthMask(GL_FALSE);
    // glDisable(GL_CULL_FACE); -- TODO: Needed for skybox rendering
    gsk_shader_use(self->shader);
    glActiveTexture(GL_TEXTURE0);

#if SKYBOX_DRAW_BLUR
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->prefilterMap->id);
    glUniform1i(glGetUniformLocation(self->shader->id, "u_draw_blur"), 1);
#else
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->cubemap->id);
    glUniform1i(glGetUniformLocation(self->shader->id, "u_draw_blur"), 0);
#endif

    gsk_gl_vertex_array_bind(self->vao);
    // glDrawArrays(GL_TRIANGLES, 0, 24);
    glDrawElements(GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
}

// HDR

gsk_Skybox *
gsk_skybox_hdr_create(gsk_Texture *hdrTexture)
{
    clearGLState();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    //  Load HDR texture
    // texture_create_hdr("../res/textures/hdr/city_night.hdr");

    // Framebuffer setup
    u32 captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // Create cubemap
    u32 skyboxCubemap;
    glGenTextures(1, &skyboxCubemap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_SRGB8,
                     512,
                     512,
                     0,
                     GL_RGB,
                     GL_FLOAT,
                     NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create irradiance map
    u32 irradianceMap;
    glGenTextures(1, &irradianceMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_RGB16F,
                     32,
                     32,
                     0,
                     GL_RGB,
                     GL_FLOAT,
                     NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create pre-filter cubemap, re-scale capture FBO tp pre-filter scale
    u32 prefilterMap;
    glGenTextures(1, &prefilterMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_RGB16F,
                     128,
                     128,
                     0,
                     GL_RGB,
                     GL_FLOAT,
                     NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(
      GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mimaps for the cubemap
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // generate BRDF texture
    u32 brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);
    // pre-allocate memory for LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Simple cube for rendering
    gsk_GlVertexArray *vao = gsk_gl_vertex_array_create();
    gsk_gl_vertex_array_bind(vao);
    gsk_GlVertexBuffer *vbo = gsk_gl_vertex_buffer_create(
      PRIM_ARR_V_CUBE, PRIM_SIZ_V_CUBE * sizeof(float));
    gsk_gl_vertex_buffer_bind(vbo);
    gsk_gl_vertex_buffer_push(vbo, 3, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_array_add_buffer(vao, vbo);
    gsk_GlIndexBuffer *ibo = gsk_gl_index_buffer_create(
      PRIM_ARR_I_CUBE, PRIM_SIZ_I_CUBE * sizeof(unsigned int));
    gsk_gl_index_buffer_bind(ibo);
    cubemapProjectionVAO = vao;

    gsk_ShaderProgram *shaderP = GSK_ASSET("gsk://shaders/hdr-cubemap.shader");
    cubemapProjectionShader    = shaderP;

    gsk_ShaderProgram *shaderConvolute =
      GSK_ASSET("gsk://shaders/hdr-convolute.shader");
    cubemapShaderConvolute = shaderConvolute;

    gsk_ShaderProgram *shaderPrefilter =
      GSK_ASSET("gsk://shaders/hdr-prefilter.shader");
    cubemapShaderPrefilter = shaderPrefilter;

    gsk_ShaderProgram *brdfShader = GSK_ASSET("gsk://shaders/hdr-brdf.shader");
    cubemapBrdfShader             = brdfShader;

    // Base skybox-render shader
    gsk_ShaderProgram *baseShader = GSK_ASSET("gsk://shaders/skybox.shader");

    cubemapProjectionFBO = captureFBO;
    cubemapProjectionRBO = captureRBO;

    // Reset
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    gsk_Skybox *ret         = malloc(sizeof(gsk_Skybox));
    ret->cubemap            = malloc(sizeof(gsk_Texture));
    ret->cubemap->id        = skyboxCubemap;
    ret->irradianceMap      = malloc(sizeof(gsk_Texture));
    ret->irradianceMap->id  = irradianceMap;
    ret->prefilterMap       = malloc(sizeof(gsk_Texture));
    ret->prefilterMap->id   = prefilterMap;
    ret->brdfLUTTexture     = malloc(sizeof(gsk_Texture));
    ret->brdfLUTTexture->id = brdfLUTTexture;
    ret->vao                = vao;
    ret->hdrTexture         = hdrTexture;
    ret->shader             = baseShader;

    // Skybox Projection
    gsk_skybox_hdr_projection(ret);

    // Return
    return ret;
}

gsk_Texture *
gsk_skybox_hdr_projection(gsk_Skybox *skybox)
{
    u32 captureFBO                     = cubemapProjectionFBO;
    u32 captureRBO                     = cubemapProjectionRBO;
    gsk_GlVertexArray *vao             = cubemapProjectionVAO;
    gsk_ShaderProgram *shaderP         = cubemapProjectionShader;
    gsk_ShaderProgram *shaderConvolute = cubemapShaderConvolute;
    gsk_ShaderProgram *shaderPrefilter = cubemapShaderPrefilter;
    gsk_ShaderProgram *brdfShader      = cubemapBrdfShader;
    gsk_Texture *hdrTexture            = skybox->hdrTexture;
    gsk_Texture *cubemapTexture        = skybox->cubemap;
    gsk_Texture *irradianceMap         = skybox->irradianceMap;
    gsk_Texture *prefilterMap          = skybox->prefilterMap;
    gsk_Texture *brdfLUTTexture        = skybox->brdfLUTTexture;

    mat4 captureProjection = GLM_MAT4_IDENTITY_INIT;
    glm_perspective(glm_rad(90.0f), 1.0f, 0.1f, 10.0f, captureProjection);

    // mat4 *captureViews = malloc(sizeof(mat4) * 6);
    mat4 captureViews[6];
    for (int i = 0; i < 6; i++)
    {
        glm_mat4_identity(captureViews[i]);
    }
    glm_lookat((vec3) {0, 0, 0},
               (vec3) {1.0f, 0.0f, 0.0f},
               (vec3) {0.0f, -1.0f, 0.0f},
               captureViews[0]);
    glm_lookat((vec3) {0, 0, 0},
               (vec3) {-1.0f, 0.0f, 0.0f},
               (vec3) {0.0f, -1.0f, 0.0f},
               captureViews[1]);
    glm_lookat((vec3) {0, 0, 0},
               (vec3) {0.0f, 1.0f, 0.0f},
               (vec3) {0.0f, 0.0f, 1.0f},
               captureViews[2]);
    glm_lookat((vec3) {0, 0, 0},
               (vec3) {0.0f, -1.0f, 0.0f},
               (vec3) {0.0f, 0.0f, -1.0f},
               captureViews[3]);
    glm_lookat((vec3) {0, 0, 0},
               (vec3) {0.0f, 0.0f, 1.0f},
               (vec3) {0.0f, -1.0f, 0.0f},
               captureViews[4]);
    glm_lookat((vec3) {0, 0, 0},
               (vec3) {0.0f, 0.0f, -1.0f},
               (vec3) {0.0f, -1.0f, 0.0f},
               captureViews[5]);

    // Convert HDR equirectangular map to cubemap equivalent
    gsk_shader_use(shaderP);
    glUniformMatrix4fv(glGetUniformLocation(shaderP->id, "projection"),
                       1,
                       GL_FALSE,
                       (float *)captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture->id);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (int i = 0; i < 6; i++)
    {
        glUniformMatrix4fv(glGetUniformLocation(shaderP->id, "view"),
                           1,
                           GL_FALSE,
                           (float *)captureViews[i]);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               cubemapTexture->id,
                               0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render 1x1 cube
        gsk_gl_vertex_array_bind(vao);
        glDrawElements(
          GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
    }
    // Convoluted Map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture->id);
    gsk_shader_use(shaderConvolute);
    glUniformMatrix4fv(glGetUniformLocation(shaderConvolute->id, "projection"),
                       1,
                       GL_FALSE,
                       (float *)captureProjection);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (int i = 0; i < 6; i++)
    {
        glUniformMatrix4fv(glGetUniformLocation(shaderConvolute->id, "view"),
                           1,
                           GL_FALSE,
                           (float *)captureViews[i]);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               irradianceMap->id,
                               0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render 1x1 cube
        gsk_gl_vertex_array_bind(vao);
        glDrawElements(
          GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
    }

    // prefilter map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture->id);
    u32 maxMipLevels = 5;
    for (u32 mip = 0; mip < maxMipLevels; ++mip)
    {
        // resize Framebuffer according to mip-level size
        u32 mipWidth  = 128 * pow(0.5, mip);
        u32 mipHeight = 128 * pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(
          GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        gsk_shader_use(shaderPrefilter);
        glUniform1f(glGetUniformLocation(shaderPrefilter->id, "u_Roughness"),
                    roughness);
        glUniformMatrix4fv(
          glGetUniformLocation(shaderPrefilter->id, "projection"),
          1,
          GL_FALSE,
          (float *)captureProjection);
        for (int i = 0; i < 6; i++)
        {
            glUniformMatrix4fv(
              glGetUniformLocation(shaderPrefilter->id, "view"),
              1,
              GL_FALSE,
              (float *)captureViews[i]);

            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   prefilterMap->id,
                                   mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // render 1x1 cube
            gsk_gl_vertex_array_bind(vao);
            glDrawElements(
              GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
        }
    }

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           brdfLUTTexture->id,
                           0);

    glViewport(0, 0, 512, 512);
    gsk_shader_use(brdfShader);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create Rectangle
    gsk_GlVertexArray *vaoRect = gsk_gl_vertex_array_create();
    gsk_gl_vertex_array_bind(vaoRect);
    float *rectPositions = prim_vert_rect();
    gsk_GlVertexBuffer *vboRect =
      gsk_gl_vertex_buffer_create(rectPositions, (2 * 3 * 4) * sizeof(float));
    gsk_gl_vertex_buffer_bind(vboRect);
    gsk_gl_vertex_buffer_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_buffer_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_array_add_buffer(vaoRect, vboRect);
    free(rectPositions);
    // Draw Quad
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    // Reset
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    skybox->cubemap = cubemapTexture;
    return skybox->cubemap;
}
