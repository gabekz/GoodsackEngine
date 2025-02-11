/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "lighting.h"

#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/device/device.h"

gsk_LightingData
gsk_lighting_initialize(u32 ubo_binding)
{
    gsk_LightingData ret;

    u32 light_ubo_id;
    if (GSK_DEVICE_API_OPENGL)
    {
        // Create lighting Uniform Buffer
        u32 light_ubo_id;
        u32 light_ubo_size = sizeof(vec3) + 4 + sizeof(vec4);
        glGenBuffers(1, &light_ubo_id);
        glBindBuffer(GL_UNIFORM_BUFFER, light_ubo_id);
        glBufferData(GL_UNIFORM_BUFFER,
                     light_ubo_size * MAX_LIGHTS,
                     NULL,
                     GL_DYNAMIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER,
                          ubo_binding,
                          light_ubo_id,
                          0,
                          light_ubo_size * MAX_LIGHTS);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        ret.ubo_id       = light_ubo_id;
        ret.ubo_size     = light_ubo_size;
        ret.ubo_binding  = ubo_binding;
        ret.total_lights = 0;
    }
    return ret;
}

void
gsk_lighting_add_light(gsk_LightingData *p_lighting_data,
                       vec3 light_position,
                       vec4 light_color)
{
    if (p_lighting_data->total_lights > MAX_LIGHTS)
    {
        LOG_ERROR("Cannot add more lights. Light limit exceeded: %d",
                  MAX_LIGHTS);
        return;
    }

    gsk_Light *p_light =
      &p_lighting_data->lights[p_lighting_data->total_lights];

    glm_vec3_copy(light_position, p_light->position);
    glm_vec4_copy(light_color, p_light->color);
    p_light->strength = 1;
    p_light->is_awake = (p_lighting_data->total_lights > 0) ? FALSE : TRUE;

#if 1 // send call to update UBO
    if (GSK_DEVICE_API_OPENGL)
    {
        // Get the starting position
        u32 ubo_offset =
          p_lighting_data->total_lights * (p_lighting_data->ubo_size);

        glBindBuffer(GL_UNIFORM_BUFFER, p_lighting_data->ubo_id);
        glBindBufferBase(GL_UNIFORM_BUFFER,
                         p_lighting_data->ubo_binding,
                         p_lighting_data->ubo_id);
        glBufferSubData(
          GL_UNIFORM_BUFFER, ubo_offset, sizeof(vec3) + 4, light_position);
        glBufferSubData(GL_UNIFORM_BUFFER,
                        ubo_offset + sizeof(vec3) + 4,
                        sizeof(vec4),
                        light_color);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
#endif

    p_lighting_data->total_lights++; // update total
}

#if 1
void
gsk_lighting_update(gsk_LightingData *p_lighting_data)
{
    for (int i = 0; i < p_lighting_data->total_lights; i++)
    {
        if (GSK_DEVICE_API_OPENGL)
        {
            // Get the starting position
            u32 ubo_offset = i * (p_lighting_data->ubo_size);

            glBindBuffer(GL_UNIFORM_BUFFER, p_lighting_data->ubo_id);
            glBindBufferBase(GL_UNIFORM_BUFFER,
                             p_lighting_data->ubo_binding,
                             p_lighting_data->ubo_id);
            glBufferSubData(GL_UNIFORM_BUFFER,
                            ubo_offset,
                            sizeof(vec3) + 4,
                            p_lighting_data->lights[i].position);
            glBufferSubData(GL_UNIFORM_BUFFER,
                            ubo_offset + sizeof(vec3) + 4,
                            sizeof(vec4),
                            p_lighting_data->lights[i].color);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
    }
}
#endif