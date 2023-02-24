#include "camera_input.h"

#include <util/logger.h>

#include <core/api/device.h>

void
camera_input(struct ComponentCamera *self, GLFWwindow *window)
{
    float *p     = GLM_VEC3_ZERO;
    float *cross = GLM_VEC3_ZERO;

    float speed = self->speed * device_getAnalytics().delta;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        // LOG_INFO("PRESS %f%f%f", self->position[0], self->position[1],
        // self->position[2]);
        glm_vec3_scale((vec3) {0.0f, 0.0f, -1.0f}, speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale((vec3) {0.0f, 0.0f, 1.0f}, speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_crossn((vec3) {0.0f, 0.0f, -1.0f}, self->axisUp, cross);
        glm_vec3_negate(cross);

        glm_vec3_scale(cross, speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_crossn((vec3) {0.0f, 0.0f, -1.0f}, self->axisUp, cross);

        glm_vec3_scale(cross, speed, p);
        glm_vec3_add(self->position, p, self->position);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm_vec3_scale((vec3) {0.0f, 1.0f, 0.0f}, speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        glm_vec3_scale((vec3) {0.0f, -1.0f, 0.0f}, speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
}
