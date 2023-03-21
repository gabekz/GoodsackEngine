#include "lighting.h"

#include <core/device/device.h>
#include <util/sysdefs.h>

Light *
light_create(float *position, float *color, LightType type)
{

    Light *ret    = malloc(sizeof(Light));
    ret->position = position;
    ret->color    = color;
    ret->type     = type;

    return ret;
}

ui32
lighting_initialize(float *lightPos, float *lightColor)
{
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
    }
    return uboLight;
}

void 
lighting_update(ui32 uboLight, float *lightPos, float *lightColor)
{
    ui32 uboLightSize = sizeof(vec3) + 4 + sizeof(vec4);
    if (DEVICE_API_OPENGL) {
        glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
        glBufferSubData(
          GL_UNIFORM_BUFFER, 0, sizeof(vec3) + 4, lightPos);
        glBufferSubData(
          GL_UNIFORM_BUFFER, sizeof(vec3) + 4, sizeof(vec4), lightColor);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}
