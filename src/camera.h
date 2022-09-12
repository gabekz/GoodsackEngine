#ifndef CAMERA_H
#define CAMERA_H

#include "gfx.h"
#include "shader.h"

#include <cglm/cglm.h>
#include <cglm/struct.h>

typedef struct _camera Camera;

struct _camera {
    float *position;
    float *axisUp;

    // MVP
    mat4 view;
    mat4 proj;

    ui32 uboId;
    int width;
    int height;

    float speed, sensitivity;
};

Camera* camera_create(int width, int height, float *position, float *up); 

void camera_send_matrix(Camera* self, float FOVdeg, float nearPlane,
  float farPlane);

void camera_input(Camera *self, GLFWwindow *window);

#endif
