#include "camera_input.h"

#include <util/logger.h>

#include <core/device/device.h>

#include <ecs/builtin/transform/transform.h>

void
camera_input(Entity cameraEntity, GLFWwindow *window)
{

    if (!ecs_has(cameraEntity, C_CAMERA) ||
        !ecs_has(cameraEntity, C_TRANSFORM)) {
        LOG_ERROR("camera_input() has incorrect component dependencies!");
        return;
    }

    struct ComponentCamera *camera       = ecs_get(cameraEntity, C_CAMERA);
    struct ComponentTransform *transform = ecs_get(cameraEntity, C_TRANSFORM);

    float *p     = GLM_VEC3_ZERO;
    float *cross = GLM_VEC3_ZERO;

    float speed = camera->speed * device_getAnalytics().delta;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        // LOG_INFO("PRESS %f%f%f", self->position[0], self->position[1],
        // self->position[2]);

        //glm_vec3_scale((vec3) {0.0f, 0.0f, -1.0f}, speed, p);
        //glm_vec3_add(transform->position, p, transform->position);

        vec3 p = GLM_VEC3_ZERO_INIT;
        glm_vec3_scale(camera->front, speed, p);
        glm_vec3_add(transform->position, p, transform->position);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        //glm_vec3_scale((vec3) {0.0f, 0.0f, 1.0f}, speed, p);
        //glm_vec3_add(transform->position, p, transform->position);
        vec3 p = GLM_VEC3_ZERO_INIT;
        glm_vec3_scale(camera->front, speed, p);
        glm_vec3_sub(transform->position, p, transform->position);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_crossn(camera->front, camera->axisUp, cross);
        //glm_vec3_negate(cross);

        glm_vec3_scale(cross, speed, p);
        glm_vec3_sub(transform->position, p, transform->position);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_crossn(camera->front, camera->axisUp, cross);
        //glm_vec3_negate(cross);

        glm_vec3_scale(cross, speed, p);
        glm_vec3_add(transform->position, p, transform->position);
    }

#if 0
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm_vec3_scale((vec3) {0.0f, 1.0f, 0.0f}, speed, p);
        glm_vec3_add(transform->position, p, transform->position);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        glm_vec3_scale((vec3) {0.0f, -1.0f, 0.0f}, speed, p);
        glm_vec3_add(transform->position, p, transform->position);
    }
#endif
}
