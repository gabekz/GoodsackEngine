/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

/* Shadowmap depth-texture pass */
#include "pass_shadowmap.h"

#include <stdio.h>
#include <stdlib.h>

#include "util/filesystem.h"
#include "util/gfx.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/material/material.h"
#include "core/graphics/shader/shader.h"

static gsk_ShaderProgram *shaderDepthMap;
static gsk_Material *materialDepthMap;
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

    /*
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
    glm_lookat((vec3) {1.0f, 2.8f, -0.2f},
               (vec3) {0.0f, 0.0f, 0.0f},
               (vec3) {0.0f, 1.0f, 0.0f},
               lightView);
    glm_mat4_mul(lightProjection, lightView, lightSpaceMatrix);
    */

    shaderDepthMap =
      gsk_shader_program_create(GSK_PATH("gsk://shaders/depth-map.shader"));
    materialDepthMap = gsk_material_create(shaderDepthMap, NULL, 0);
}

void
shadowmap_bind()
{
    gsk_shader_use(shaderDepthMap);
    glUniformMatrix4fv(
      glGetUniformLocation(shaderDepthMap->id, "u_LightSpaceMatrix"),
      1,
      GL_FALSE,
      (float *)lightSpaceMatrix);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Bind the shadowmap to texture slot 8
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
}

void
shadowmap_update(vec3 lightPosition, ShadowmapOptions options)
{
    glm_mat4_zero(lightSpaceMatrix);
    mat4 lightProjection = GLM_MAT4_ZERO_INIT;
    mat4 lightView       = GLM_MAT4_ZERO_INIT;
    float nearPlane = options.nearPlane, farPlane = options.farPlane;
    float camSize = options.camSize;
    glm_ortho(-camSize,
              camSize,
              -camSize,
              camSize,
              nearPlane,
              farPlane,
              lightProjection);
    glm_lookat(lightPosition,
               (vec3) {0.0f, 0.0f, 0.0f}, // center
               (vec3) {0.0f, 1.0f, 0.0f}, // up-axis
               lightView);
    glm_mat4_mul(lightProjection, lightView, lightSpaceMatrix);
}

void
shadowmap_bind_texture()
{
    // Bind the shadowmap to texture slot 6
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
}

gsk_Material *
shadowmap_getMaterial()
{
    return materialDepthMap;
}

ui32
shadowmap_getTexture()
{
    return depthMapTexture;
}

vec4 *
shadowmap_getMatrix()
{
    return lightSpaceMatrix;
}
