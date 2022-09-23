#include "camera.h"
#include <util/gfx.h>

void camera_input(struct ComponentCamera *self, GLFWwindow *window) {
    float *p = GLM_VEC3_ZERO;
    float *cross = GLM_VEC3_ZERO;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale((vec3){0.0f, 0.0f, -1.0f}, self->speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale((vec3){0.0f, 0.0f, 1.0f}, self->speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_crossn((vec3){0.0f, 0.0f, -1.0f}, self->axisUp, cross);
        glm_vec3_negate(cross);

        glm_vec3_scale(cross, self->speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_crossn((vec3){0.0f, 0.0f, -1.0f}, self->axisUp, cross);

        glm_vec3_scale(cross, self->speed, p);
        glm_vec3_add(self->position, p, self->position);
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm_vec3_scale((vec3){0.0f, 1.0f, 0.0f}, self->speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        glm_vec3_scale((vec3){0.0f, -1.0f, 0.0f}, self->speed, p);
        glm_vec3_add(self->position, p, self->position);
    }
}
