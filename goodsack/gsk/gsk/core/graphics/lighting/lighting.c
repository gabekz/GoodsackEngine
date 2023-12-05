/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "lighting.h"

#include "util/sysdefs.h"
#include "core/device/device.h"

gsk_Light *
gsk_lighting_initialize(vec3 lightPos, vec4 lightColor)
{
    gsk_Light *ret = malloc(sizeof(gsk_Light));
    glm_vec3_copy(lightPos, ret->position);
    glm_vec4_copy(lightColor, ret->color);

    ret->type     = Directional;
    ret->strength = 4; // TODO: GNK

    ui32 uboLight;
    if (DEVICE_API_OPENGL) {
        ui32 uboLightSize = sizeof(vec3) + 4 + sizeof(vec4);
        glGenBuffers(1, &uboLight);
        glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
        glBufferData(GL_UNIFORM_BUFFER, uboLightSize, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, uboLight, 0, uboLightSize);
        // Send lighting data to UBO
        glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vec3) + 4, lightPos);
        glBufferSubData(
          GL_UNIFORM_BUFFER, sizeof(vec3) + 4, sizeof(vec4), lightColor);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        ret->ubo = uboLight;
    }
    return ret;
}

void
gsk_lighting_update(gsk_Light *light, vec3 lightPos, vec4 lightColor)
{
    ui32 uboLightSize = sizeof(vec3) + 4 + sizeof(vec4);
    if (DEVICE_API_OPENGL) {
        glBindBuffer(GL_UNIFORM_BUFFER, light->ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vec3) + 4, lightPos);
        glBufferSubData(
          GL_UNIFORM_BUFFER, sizeof(vec3) + 4, sizeof(vec4), lightColor);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}
