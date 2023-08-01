/** camera.c
TODO:
- CameraComponent needs to be split into several parts:

// FOV, Near/Far, Projection
- CameraComponent
// MouseLook, sensitivity
- CameraLookComponent

// Trauma, etc.
- CameraShakeComponent

// Movement
- FlyControlComponent
**/

#include "camera.h"

#include <entity/v1/builtin/camera/camera_input.h>
#include <entity/v1/builtin/transform/transform.h>

#include <string.h>

#include <core/device/device.h>
#include <entity/v1/ecs.h>
#include <util/gfx.h>

#define CAMERA_SHAKE            1
#define CAMERA_SENSITIVITY_DIVS 10.0f

#if CAMERA_SHAKE
static float s_shake = 0.00f;

static float
_noise(int x, int y)
{
    int n;

    n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0 - ((n * ((n * n * 15731) + 789221) + 1376312589) & 0x7fffffff) /
                    1073741824.0);
}
#endif // CAMERA_SHAKE

static void
_initialize_shader_data(struct ComponentCamera *camera)
{
    // initialize default view and projection matrices
    mat4 m4i = GLM_MAT4_IDENTITY_INIT;
    // glm_mat4_copy(m4i, camera->uniform.view);
    // glm_mat4_copy(m4i, camera->uniform.proj);
    glm_mat4_copy(m4i, camera->view);
    glm_mat4_copy(m4i, camera->proj);

// Create UBO for camera data
#if 0
    ui32 uboSize = 4 + sizeof(vec3) + (2 * sizeof(mat4));
    if (DEVICE_API_OPENGL) {
        ui32 uboId;
        glGenBuffers(1, &uboId);
        glBindBuffer(GL_UNIFORM_BUFFER, uboId);
        glBufferData(GL_UNIFORM_BUFFER, uboSize, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboId, 0, uboSize);
        camera->uboId = uboId;
}
    else if (DEVICE_API_VULKAN)
    {
        /*
        VulkanDeviceContext *context = e.ecs->renderer->vulkanDevice;

        vulkan_uniform_buffer_create(context->physicalDevice, context->device,
                &camera->uniformBuffer,
                &camera->uniformBufferMemory,
                &camera->uniformBufferMapped
        );
        */
    }
#endif
}

static void
_upload_shader_data(Entity e,
                    struct ComponentCamera *camera,
                    struct ComponentTransform *transform)
{
    if (DEVICE_API_OPENGL) {

        ui32 renderLayer = 0; // TODO: render layer
        // Get the starting position
        ui32 ubo_index = renderLayer * e.ecs->renderer->camera_data.uboSize;

        glBindBuffer(GL_UNIFORM_BUFFER, e.ecs->renderer->camera_data.uboId);
        glBufferSubData(
          GL_UNIFORM_BUFFER, 0, sizeof(vec4), transform->position);
        glBufferSubData(GL_UNIFORM_BUFFER,
                        ubo_index + sizeof(vec4),
                        sizeof(mat4),
                        camera->proj);
        glBufferSubData(GL_UNIFORM_BUFFER,
                        ubo_index + sizeof(mat4) + sizeof(vec4),
                        sizeof(mat4),
                        camera->view);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    } else if (DEVICE_API_VULKAN) {

        // TEST (while we don't have direct descriptor sets for objects)
        mat4 p = GLM_MAT4_IDENTITY_INIT;
        // glm_mat4_copy(p, camera->uniform.model);
        glm_mat4_copy(p, camera->model);
        // glm_rotate(camera->uniform.model, glm_rad(180.0f), (vec3){0, 1, 0});
        // glm_rotate(camera->uniform.model, glm_rad(45.0f), (vec3){1, 0, 1});
        camera->proj[1][1] *= -1;
        // camera->uniform.proj[1][1] *= -1;

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

    // TODO: remove camera-> screenWidth/screenHeight
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

    // Camera Look //

    if (!(ecs_has(e, C_CAMERALOOK))) return;
    struct ComponentCameraLook *cameraLook = ecs_get(e, C_CAMERALOOK);

    cameraLook->lastX = e.ecs->renderer->windowWidth / 2;
    cameraLook->lastY = e.ecs->renderer->windowHeight / 2;
    cameraLook->yaw   = -90.0f;
    cameraLook->pitch = 0;

    cameraLook->firstMouse = TRUE;
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_CAMERA))) return;
    if (!(ecs_has(e, C_CAMERALOOK))) return;
    if (!(ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCamera *camera = ecs_get(e, C_CAMERA);
    struct ComponentCameraLook *cameraLook =
      ecs_get(e, C_CAMERALOOK); // TODO: Move away

    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    Input input = device_getInput();
    double cntX, cntY;

    if (input.cursor_state.is_locked == TRUE) {

        cntX = input.cursor_position[0];
        cntY = input.cursor_position[1];

        if (cameraLook->firstMouse) {
            cameraLook->lastX = cntX;
            cameraLook->lastY = cntY;

            cameraLook->firstMouse = FALSE;
        }

    } else {
        cntX                   = cameraLook->lastX;
        cntY                   = cameraLook->lastY;
        cameraLook->firstMouse = TRUE;
    }

    float xOffset = cntX - cameraLook->lastX;
    float yOffset = cameraLook->lastY - cntY;

    cameraLook->lastX = cntX;
    cameraLook->lastY = cntY;

    const float sensitivity = cameraLook->sensitivity / CAMERA_SENSITIVITY_DIVS;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    cameraLook->yaw += xOffset;
    cameraLook->pitch += yOffset;

#if CAMERA_SHAKE
    // float randomFloat = ((float)rand() / (float)(RAND_MAX)) * 2 - 1;
    float seed    = 255.0f;
    float shakeCO = 0.5f * s_shake * _noise(seed, glfwGetTime() * 50.0f);
#endif // CAMERA_SHAKE

    // Clamp pitch
    if (cameraLook->pitch > 89.0f) cameraLook->pitch = 89.0f;
    if (cameraLook->pitch < -89.0f) cameraLook->pitch = -89.0f;

    // Calculate camera direction
    vec3 camDirection = GLM_VEC3_ZERO_INIT;
#if CAMERA_SHAKE
    camDirection[0] = cos(glm_rad(cameraLook->yaw + shakeCO + 1)) *
                      cos(glm_rad(cameraLook->pitch + shakeCO));
    camDirection[1] = sin(glm_rad(cameraLook->pitch + shakeCO));
    camDirection[2] = sin(glm_rad(cameraLook->yaw + shakeCO + 1)) *
                      cos(glm_rad(cameraLook->pitch));
#else
    camDirection[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    camDirection[1] = sin(glm_rad(camera->pitch));
    camDirection[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
#endif // CAMERA_SHAKE

    glm_vec3_normalize_to(camDirection, camera->front);

    // copy front to orientation
    float camDirectionDeg[3] = {glm_deg(camera->front[1]),
                                glm_deg(-camera->front[0]),
                                glm_deg(-camera->front[2])};
    // glm_vec3_copy(camDirectionDeg, transform->orientation);
    // glm_mat4_inv(camera->model, transform->model);

    // Process Camera Input as long as the cursor is locked
    if (input.cursor_state.is_locked == TRUE) {
        camera_input(e, e.ecs->renderer->window);
    }

    // glm_vec3_add(transform->position, camDirection, camera->front);

    vec3 cameraUp = GLM_VEC3_ZERO_INIT;
    glm_cross(camera->axisUp, camera->front, cameraUp);
    glm_vec3_normalize(cameraUp);

    vec3 p = GLM_VEC3_ZERO_INIT;
    glm_vec3_add(transform->position, camera->front, p);

    // MVP: view
    glm_lookat(transform->position, p, camera->axisUp, camera->view);
    glm_mat4_inv(camera->view, transform->model);
    // glm_mat4_copy(camera->view, transform->model);

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

#if CAMERA_SHAKE
    if (s_shake > 0) {
        s_shake -= 3 * device_getAnalytics().delta;
    } else if (s_shake <= 0) {
        s_shake = 0;
    }

    if (glfwGetKey(e.ecs->renderer->window, GLFW_KEY_P) == GLFW_PRESS) {
        s_shake += 0.165f;
    }
    if (glfwGetKey(e.ecs->renderer->window, GLFW_KEY_O) == GLFW_PRESS) {
        s_shake = 2;
    }
#endif // CAMERA_SHAKE
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
