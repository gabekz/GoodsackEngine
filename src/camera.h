#ifndef CAMERA_H
#define CAMERA_H

#include "shader.h"

#include <cglm/cglm.h>
#include <cglm/struct.h>

#include <util/gfx.h>

typedef struct _camera Camera;

struct _camera {
    si32 width, height;
    ui32 uboId;
    float *position, *axisUp;
    float speed, sensitivity;
    mat4 view, proj;
};

Camera* camera_create(int width, int height, float *position, float *up); 

void camera_send_matrix(Camera* self, float FOVdeg, float nearPlane,
  float farPlane);

void camera_input(Camera *self, GLFWwindow *window);

#endif
