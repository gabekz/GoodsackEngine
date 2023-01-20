#include "skybox.h"

#include <core/api/opengl/opengl.h>
#include <core/shader/shader.h>
#include <model/model.h>
#include <model/primitives.h>

static ui32 cubemapProjectionFBO;
static VAO *cubemapProjectionVAO;
static ShaderProgram *cubemapProjectionShader;

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
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->cubemap->id);

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

    cubemapProjectionFBO = captureFBO;

    // Reset
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Return
    Skybox *ret      = malloc(sizeof(Skybox));
    ret->cubemap     = malloc(sizeof(Texture));
    ret->cubemap->id = skyboxCubemap;
    ret->vao         = vao;
    ret->hdrTexture  = hdrTexture;
    return ret;
}

Texture *
skybox_hdr_projection(Skybox *skybox)
{
    ui32 captureFBO         = cubemapProjectionFBO;
    VAO *vao                = cubemapProjectionVAO;
    ShaderProgram *shaderP  = cubemapProjectionShader;
    Texture *hdrTexture     = skybox->hdrTexture;
    Texture *cubemapTexture = skybox->cubemap;

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
    glUniform1i(glGetUniformLocation(shaderP->id, "skybox"), 0);

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

    // Reset
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    skybox->cubemap = cubemapTexture;
    return skybox->cubemap;
}
