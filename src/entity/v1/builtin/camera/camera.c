#include "camera.h"
#include <entity/v1/builtin/camera/camera_input.h>
#include <entity/v1/builtin/transform/transform.h>

#include <string.h>

#include <core/device/device.h>
#include <entity/v1/ecs.h>
#include <util/gfx.h>

static void
_initialize_shader_data(struct ComponentCamera *camera)
{
    // initialize default view and projection matrices
    mat4 m4i = GLM_MAT4_IDENTITY_INIT;
    //glm_mat4_copy(m4i, camera->uniform.view);
    //glm_mat4_copy(m4i, camera->uniform.proj);
    glm_mat4_copy(m4i, camera->view);
    glm_mat4_copy(m4i, camera->proj);

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
}

static void
_upload_shader_data(Entity e,
                    struct ComponentCamera *camera,
                    struct ComponentTransform *transform)
{
    if (DEVICE_API_OPENGL) {
        glBindBuffer(GL_UNIFORM_BUFFER, camera->uboId);
        glBufferSubData(
          GL_UNIFORM_BUFFER, 0, sizeof(vec3) + 4, transform->position);
        glBufferSubData(GL_UNIFORM_BUFFER,
                        sizeof(vec3) + 4,
                        sizeof(mat4),
                        //camera->uniform.proj);
                        camera->proj);
        glBufferSubData(GL_UNIFORM_BUFFER,
                        sizeof(mat4) + sizeof(vec3) + 4,
                        sizeof(mat4),
                        //camera->uniform.view);
                        camera->view);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    } else if (DEVICE_API_VULKAN) {

        // TEST (while we don't have direct descriptor sets for objects)
        mat4 p = GLM_MAT4_IDENTITY_INIT;
        //glm_mat4_copy(p, camera->uniform.model);
        glm_mat4_copy(p, camera->model);
        // glm_rotate(camera->uniform.model, glm_rad(180.0f), (vec3){0, 1, 0});
        // glm_rotate(camera->uniform.model, glm_rad(45.0f), (vec3){1, 0, 1});
        camera->proj[1][1] *= -1;
        //camera->uniform.proj[1][1] *= -1;

        #if !(USING_GENERATED_COMPONENTS)
        memcpy(
          e.ecs->renderer->vulkanDevice
            ->uniformBuffersMapped[e.ecs->renderer->vulkanDevice->currentFrame],
          &camera->uniform,
          sizeof(camera->uniform));
        #endif
    }
}

static void
init(Entity e)
{
    if (!(ecs_has(e, C_CAMERA))) return;
    if (!(ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCamera *camera       = ecs_get(e, C_CAMERA);
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    camera->screenWidth  = e.ecs->renderer->windowWidth;
    camera->screenHeight = e.ecs->renderer->windowHeight;

    // Defaults
    if (camera->fov <= 0) { camera->fov = 45.0f; }
    if (camera->nearZ <= 0) { camera->nearZ = 0.1f; }
    if (camera->farZ <= 0) { camera->farZ = 100.0f; }

    // Create camera UBO
    _initialize_shader_data(camera);

    float *center = GLM_VEC3_ZERO; // position + orientation _v
    glm_vec3_mul(transform->position, (vec3) {0.0f, 0.0f, -1.0f}, center);

    camera->lastX = e.ecs->renderer->windowWidth / 2;
    camera->lastY = e.ecs->renderer->windowHeight / 2;
    camera->yaw   = -90.0f;
    camera->pitch = 0;

    camera->firstMouse = TRUE;
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_CAMERA))) return;
    if (!(ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCamera *camera       = ecs_get(e, C_CAMERA);
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    Input input = device_getInput();
    double cntX, cntY;
    if (input.holding_right_button == TRUE) {

        cntX = input.cursor_position[0];
        cntY = input.cursor_position[1];

        if (camera->firstMouse) {
            camera->lastX = cntX;
            camera->lastY = cntY;

            camera->firstMouse = FALSE;
        }

    } else {
        cntX               = camera->lastX;
        cntY               = camera->lastY;
        camera->firstMouse = TRUE;
    }

    float xOffset = cntX - camera->lastX;
    float yOffset = camera->lastY - cntY;

    camera->lastX = cntX;
    camera->lastY = cntY;

    const float sensitivity = 0.1f;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    camera->yaw += xOffset;
    camera->pitch += yOffset;

    // Clamp pitch
    if (camera->pitch > 89.0f) camera->pitch = 89.0f;
    if (camera->pitch < -89.0f) camera->pitch = -89.0f;

    // Calculate camera direction
    vec3 camDirection = GLM_VEC3_ZERO_INIT;
    camDirection[0]   = cos(glm_rad(camera->yaw) * cos(glm_rad(camera->pitch)));
    camDirection[1]   = sin(glm_rad(camera->pitch));
    camDirection[2]   = sin(glm_rad(camera->yaw) * cos(glm_rad(camera->pitch)));
    glm_vec3_normalize_to(camDirection, camera->front);

    // Process Camera Input
    camera_input(e, e.ecs->renderer->window);

    // glm_vec3_add(transform->position, camDirection, camera->front);

    vec3 cameraUp = GLM_VEC3_ZERO_INIT;
    glm_cross(camera->axisUp, camera->front, cameraUp);
    glm_vec3_normalize(cameraUp);

    vec3 p = GLM_VEC3_ZERO_INIT;
    glm_vec3_add(transform->position, camera->front, p);

    // MVP: view
    glm_lookat(transform->position, p, camera->axisUp, camera->view);

    // LOG_INFO("\tPitch: %f\tYaw:%f", camera->pitch, camera->yaw);

    float aspectRatio =
      (float)camera->screenWidth / (float)camera->screenHeight;
    // MVP: projection
    glm_perspective(glm_rad(camera->fov),
                    aspectRatio,
                    camera->nearZ,
                    camera->farZ,
                    camera->proj);
    // camera->uniform.proj[1][1] *= -1;

    // Update camera UBO
    _upload_shader_data(e, camera, transform);
}

void
s_camera_init(ECS *ecs)
{
    //_ECS_DECL_COMPONENT(ecs, C_CAMERA, sizeof(struct ComponentCamera));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = NULL,
                          .update  = (ECSSubscriber)update,
                        }));
}
