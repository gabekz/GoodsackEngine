#include "camera.h"

#include <stdlib.h>

Camera* camera_create(int width, int height, float *position, float *up) {

    Camera* cam = malloc(sizeof(Camera));
    cam->width      = width;
    cam->height     = height;
    cam->position   = position;
    cam->axisUp     = up;
    cam->speed      = 0.05f;

    // initialize default view and projection matrixes
    mat4 m4init = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_copy(m4init, cam->view);
    glm_mat4_copy(m4init, cam->proj);

    /*
    cam->orientation = orientation;
    cam->axisUp = axisUp;
    cam->speed = speed;
    cam->sensitivity = speed;
    */

    return cam;
}

// Todo: should have separate function for lookat() and such for translations.
void camera_send_matrix(Camera* self, float FOVdeg, float nearPlane,
  float farPlane) {

    float *axisUp = self->axisUp;
    float *center = GLM_VEC3_ZERO; // position + orientation _v
    glm_vec3_mul(self->position, (vec3){0.0f, 0.0f, -1.0f}, center);

// lookat [Todo: should be something like camera_update ??]
    glm_lookat(self->position, center, axisUp, self->view);
    //glm_translate(view, (vec3){0.0f, -0.5f, -2.0f}); ~ old but perhaps better

    float aspectRatio = (float)self->width / (float)self->height;
    glm_perspective(glm_rad(FOVdeg), aspectRatio, nearPlane, farPlane, self->proj);
}

void camera_matrix(Camera* self, unsigned int shaderId, const char* uniform) {

    mat4 val = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(self->proj, self->view, val);

    glUniformMatrix4fv(glGetUniformLocation(shaderId, uniform), 1,
      GL_FALSE, (float *)val);
}

void camera_input(Camera *self, GLFWwindow *window) {
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

