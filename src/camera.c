#include "camera.h"

#include <stdlib.h>
#include <util/sysdefs.h>

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

// Create UBO for camera data
    ui32 uboId;
    ui32 uboSize = 4 + sizeof(vec3) + (2 * sizeof(mat4));
    glGenBuffers(1, &uboId);
    glBindBuffer(GL_UNIFORM_BUFFER, uboId);
    glBufferData(GL_UNIFORM_BUFFER, uboSize, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboId, 0, uboSize);
    cam->uboId = uboId;

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

// Update camera UBO
        glBindBuffer(GL_UNIFORM_BUFFER, self->uboId);
        glBufferSubData(GL_UNIFORM_BUFFER,
            0, sizeof(vec3) + 4,
            self->position);
        glBufferSubData(GL_UNIFORM_BUFFER,
            sizeof(vec3) + 4, sizeof(mat4),
            self->proj);
        glBufferSubData(GL_UNIFORM_BUFFER,
            sizeof(mat4) + sizeof(vec3) + 4, sizeof(mat4),
            self->view);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
