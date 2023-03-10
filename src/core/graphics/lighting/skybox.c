#include "skybox.h"

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/mesh/mesh.h>
#include <core/graphics/mesh/primitives.h>
#include <core/graphics/shader/shader.h>

static ui32 cubemapProjectionFBO;
static ui32 cubemapProjectionRBO;
static VAO *cubemapProjectionVAO;
static ShaderProgram *cubemapProjectionShader;
static ShaderProgram *cubemapShaderConvolute;
static ShaderProgram *cubemapShaderPrefilter;
static ShaderProgram *cubemapBrdfShader;

Skybox *
skybox_create(Texture *cubemap)
{
    Skybox *ret  = malloc(sizeof(Skybox));
    ret->cubemap = cubemap;

    VAO *vao = vao_create();
    ret->vao = vao;

    VBO *vbo = vbo_create(PRIM_ARR_V_CUBE, PRIM_SIZ_V_CUBE * sizeof(float));
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vbo);
    IBO *ibo =
      ibo_create(PRIM_ARR_I_CUBE, PRIM_SIZ_I_CUBE * sizeof(unsigned int));
    ibo_bind(ibo);
    free(vbo);
    ShaderProgram *shader =
      shader_create_program("../res/shaders/skybox.shader");
    ret->shader = shader;

    return ret;
}

void
skybox_draw(Skybox *self)
{
    // glDepthMask(GL_FALSE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->prefilterMap->id);

    shader_use(self->shader);
    vao_bind(self->vao);
    // glDrawArrays(GL_TRIANGLES, 0, 24);
    glDrawElements(GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
}

// HDR

Skybox *
skybox_hdr_create()
{
    clearGLState();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    //  Load HDR texture
    Texture *hdrTexture =
      texture_create_hdr("../res/textures/hdr/sky_cloudy_ref.hdr");
    // texture_create_hdr("../res/textures/hdr/city_night.hdr");

    // Framebuffer setup
    ui32 captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // Create cubemap
    ui32 skyboxCubemap;
    glGenTextures(1, &skyboxCubemap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_RGB16F,
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
    ui32 irradianceMap;
    glGenTextures(1, &irradianceMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (int i = 0; i < 6; i++) {
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
    ui32 prefilterMap;
    glGenTextures(1, &prefilterMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (int i = 0; i < 6; i++) {
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
    ui32 brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);
    // pre-allocate memory for LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Simple cube for rendering
    VAO *vao = vao_create();
    vao_bind(vao);
    VBO *vbo = vbo_create(PRIM_ARR_V_CUBE, PRIM_SIZ_V_CUBE * sizeof(float));
    vbo_bind(vbo);
    vbo_push(vbo, 3, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vao, vbo);
    IBO *ibo =
      ibo_create(PRIM_ARR_I_CUBE, PRIM_SIZ_I_CUBE * sizeof(unsigned int));
    ibo_bind(ibo);
    cubemapProjectionVAO = vao;

    ShaderProgram *shaderP =
      shader_create_program("../res/shaders/hdr-cubemap.shader");
    cubemapProjectionShader = shaderP;

    ShaderProgram *shaderConvolute =
      shader_create_program("../res/shaders/hdr-convolute.shader");
    cubemapShaderConvolute = shaderConvolute;

    ShaderProgram *shaderPrefilter =
      shader_create_program("../res/shaders/hdr-prefilter.shader");
    cubemapShaderPrefilter = shaderPrefilter;

    ShaderProgram *brdfShader =
      shader_create_program("../res/shaders/hdr-brdf.shader");
    cubemapBrdfShader = brdfShader;

    cubemapProjectionFBO = captureFBO;
    cubemapProjectionRBO = captureRBO;

    // Reset
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Return
    Skybox *ret             = malloc(sizeof(Skybox));
    ret->cubemap            = malloc(sizeof(Texture));
    ret->cubemap->id        = skyboxCubemap;
    ret->irradianceMap      = malloc(sizeof(Texture));
    ret->irradianceMap->id  = irradianceMap;
    ret->prefilterMap       = malloc(sizeof(Texture));
    ret->prefilterMap->id   = prefilterMap;
    ret->brdfLUTTexture     = malloc(sizeof(Texture));
    ret->brdfLUTTexture->id = brdfLUTTexture;
    ret->vao                = vao;
    ret->hdrTexture         = hdrTexture;
    return ret;
}

Texture *
skybox_hdr_projection(Skybox *skybox)
{
    ui32 captureFBO                = cubemapProjectionFBO;
    ui32 captureRBO                = cubemapProjectionRBO;
    VAO *vao                       = cubemapProjectionVAO;
    ShaderProgram *shaderP         = cubemapProjectionShader;
    ShaderProgram *shaderConvolute = cubemapShaderConvolute;
    ShaderProgram *shaderPrefilter = cubemapShaderPrefilter;
    ShaderProgram *brdfShader      = cubemapBrdfShader;
    Texture *hdrTexture            = skybox->hdrTexture;
    Texture *cubemapTexture        = skybox->cubemap;
    Texture *irradianceMap         = skybox->irradianceMap;
    Texture *prefilterMap          = skybox->prefilterMap;
    Texture *brdfLUTTexture        = skybox->brdfLUTTexture;

    mat4 captureProjection = GLM_MAT4_IDENTITY_INIT;
    glm_perspective(glm_rad(90.0f), 1.0f, 0.1f, 10.0f, captureProjection);

    // mat4 *captureViews = malloc(sizeof(mat4) * 6);
    mat4 captureViews[6];
    for (int i = 0; i < 6; i++) {
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
    shader_use(shaderP);
    glUniformMatrix4fv(glGetUniformLocation(shaderP->id, "projection"),
                       1,
                       GL_FALSE,
                       (float *)captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture->id);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (int i = 0; i < 6; i++) {
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
        vao_bind(vao);
        glDrawElements(
          GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
    }
    // Convoluted Map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture->id);
    shader_use(shaderConvolute);
    glUniformMatrix4fv(glGetUniformLocation(shaderConvolute->id, "projection"),
                       1,
                       GL_FALSE,
                       (float *)captureProjection);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (int i = 0; i < 6; i++) {
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
        vao_bind(vao);
        glDrawElements(
          GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
    }

    // prefilter map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture->id);
    ui32 maxMipLevels = 5;
    for (ui32 mip = 0; mip < maxMipLevels; ++mip) {
        // resize Framebuffer according to mip-level size
        ui32 mipWidth  = 128 * pow(0.5, mip);
        ui32 mipHeight = 128 * pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(
          GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        shader_use(shaderPrefilter);
        glUniform1f(glGetUniformLocation(shaderPrefilter->id, "u_Roughness"),
                    roughness);
        glUniformMatrix4fv(
          glGetUniformLocation(shaderPrefilter->id, "projection"),
          1,
          GL_FALSE,
          (float *)captureProjection);
        for (int i = 0; i < 6; i++) {
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
            vao_bind(vao);
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
    shader_use(brdfShader);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create Rectangle
    VAO *vaoRect = vao_create();
    vao_bind(vaoRect);
    float *rectPositions = prim_vert_rect();
    VBO *vboRect = vbo_create(rectPositions, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vboRect);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vboRect, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(vaoRect, vboRect);
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
