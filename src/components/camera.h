#ifndef H_C_CAMERA
#define H_C_CAMERA

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

struct ComponentCamera {
    vec3 position, axisUp;
    float speed, sensitivity;
    ui32 uboId;
    struct {
        mat4 view, proj;
    } mvp;
    struct {
        int width, height;
    } screen;
};

void camera_input(struct ComponentCamera*, GLFWwindow*);

#endif // H_C_CAMERA
