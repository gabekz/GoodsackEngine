#include "camera.h"
#include <components/camera/camera_input.h>

#include <string.h>

#include <core/api/device.h>
#include <ecs/ecs.h>
#include <util/gfx.h>

static void
init(Entity e)
{
    if (!(ecs_has(e, C_CAMERA))) return;
    struct ComponentCamera *camera = ecs_get(e, C_CAMERA);

    camera->screen.width  = e.ecs->renderer->windowWidth;
    camera->screen.height = e.ecs->renderer->windowHeight;
    // camera->screen.width = 640;
    // camera->screen.height = 480;

    // Defaults
    if (camera->fov <= 0) { camera->fov = 45.0f; }
    if (camera->clipping.nearZ <= 0) { camera->clipping.nearZ = 0.1f; }
    if (camera->clipping.farZ <= 0) { camera->clipping.farZ = 100.0f; }

    // initialize default view and projection matrices
    mat4 m4i = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_copy(m4i, camera->uniform.view);
    glm_mat4_copy(m4i, camera->uniform.proj);

    // Create UBO for camera data
    ui32 uboSize = 4 + sizeof(vec3) + (2 * sizeof(mat4));
    if (DEVICE_API_OPENGL) {
        ui32 uboId;
        glGenBuffers(1, &uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferData(GL_UNIFORM_BUFFER, uboSize, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboId, 0, uboSize);
        camera->uboId = uboId;
    } else if (DEVICE_API_VULKAN) {
        /*
        VulkanDeviceContext *context = e.ecs->renderer->vulkanDevice;

        vulkan_uniform_buffer_create(context->physicalDevice, context->device,
                &camera->uniformBuffer,
                &camera->uniformBufferMemory,
                &camera->uniformBufferMapped
        );
        */
    }

    float *axisUp = camera->axisUp;
    float *center = GLM_VEC3_ZERO; // position + orientation _v
    glm_vec3_mul(camera->position, (vec3) {0.0f, 0.0f, -1.0f}, center);
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_CAMERA))) return;
    struct ComponentCamera *camera = ecs_get(e, C_CAMERA);

    float *axisUp = camera->axisUp;
    glm_vec3_zero(camera->center);
    glm_vec3_mul(camera->position, (vec3) {0.0f, 0.0f, -1.0f}, camera->center);
    glm_lookat(camera->position, camera->center, axisUp, camera->uniform.view);

    float aspectRatio =
      (float)camera->screen.width / (float)camera->screen.height;
    glm_perspective(glm_rad(camera->fov),
                    aspectRatio,
                    camera->clipping.nearZ,
                    camera->clipping.farZ,
                    camera->uniform.proj);
    // camera->uniform.proj[1][1] *= -1;

    // Update camera UBO
    if (DEVICE_API_OPENGL) {
        glBindBuffer(GL_UNIFORM_BUFFER, camera->uboId);
        glBufferSubData(
          GL_UNIFORM_BUFFER, 0, sizeof(vec3) + 4, camera->position);
        glBufferSubData(GL_UNIFORM_BUFFER,
                        sizeof(vec3) + 4,
                        sizeof(mat4),
                        camera->uniform.proj);
        glBufferSubData(GL_UNIFORM_BUFFER,
                        sizeof(mat4) + sizeof(vec3) + 4,
                        sizeof(mat4),
                        camera->uniform.view);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    } else if (DEVICE_API_VULKAN) {

        // TEST (while we don't have direct descriptor sets for objects)
        mat4 p = GLM_MAT4_IDENTITY_INIT;
        glm_mat4_copy(p, camera->uniform.model);
        // glm_rotate(camera->uniform.model, glm_rad(180.0f), (vec3){0, 1, 0});
        // glm_rotate(camera->uniform.model, glm_rad(45.0f), (vec3){1, 0, 1});
        camera->uniform.proj[1][1] *= -1;

        memcpy(
          e.ecs->renderer->vulkanDevice
            ->uniformBuffersMapped[e.ecs->renderer->vulkanDevice->currentFrame],
          &camera->uniform,
          sizeof(camera->uniform));
    }

    // Process Camera Input
    camera_input(camera, e.ecs->renderer->window);
}

void
s_camera_init(ECS *ecs)
{
    ecs_component_register(ecs, C_CAMERA, sizeof(struct ComponentCamera));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = NULL,
                          .update  = (ECSSubscriber)update,
                        }));
}
