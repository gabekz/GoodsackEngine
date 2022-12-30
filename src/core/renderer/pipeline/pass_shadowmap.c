/* Shadowmap depth-texture pass */
#include "pass_shadowmap.h"

#include <stdio.h>
#include <stdlib.h>

#include <core/api/opengl/glbuffer.h>
#include <core/shader/shader.h>
#include <model/material.h>

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

static ShaderProgram *shaderDepthMap;
static Material *materialDepthMap;
static ui32 depthMapFBO;
static ui32 depthMapTexture;
static mat4 lightSpaceMatrix;

void
shadowmap_init()
{
    // Create the depthmap
    glGenFramebuffers(1, &depthMapFBO);
    // Texture
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH,
                 SHADOW_HEIGHT,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_FLOAT,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glm_mat4_zero(lightSpaceMatrix);
    mat4 lightProjection = GLM_MAT4_ZERO_INIT;
    mat4 lightView       = GLM_MAT4_ZERO_INIT;
    float nearPlane = 0.5f, farPlane = 7.5f;
    float camSize = 10.0f;
    glm_ortho(-camSize,
              camSize,
              -camSize,
              camSize,
              nearPlane,
              farPlane,
              lightProjection);
    glm_lookat((vec3) {1.0f, 1.0f, 1.0f},
               (vec3) {0.0f, 0.0f, 0.0f},
               (vec3) {0.0f, 1.0f, 0.0f},
               lightView);
    glm_mat4_mul(lightProjection, lightView, lightSpaceMatrix);

    shaderDepthMap = shader_create_program("../res/shaders/depth-map.shader");

    materialDepthMap = material_create(shaderDepthMap, NULL, 0);
}

void
shadowmap_bind()
{
    shader_use(shaderDepthMap);
    glUniformMatrix4fv(
      glGetUniformLocation(shaderDepthMap->id, "u_LightSpaceMatrix"),
      1,
      GL_FALSE,
      (float *)lightSpaceMatrix);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Bind the shadowmap to texture slot 6
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
}

void
shadowmap_bind_texture()
{
    // Bind the shadowmap to texture slot 6
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
}

Material *
shadowmap_getMaterial()
{
    return materialDepthMap;
}

vec4 *
shadowmap_getMatrix()
{
    return lightSpaceMatrix;
}
