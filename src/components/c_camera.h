#ifndef H_C_CAMERA
#define H_C_CAMERA

#include <util/sysdefs.h>
#include <util/maths.h>

struct ComponentCamera {

    float *position, *axisUp;
    float speed, sensitivity;
    ui32 uboId;

    struct {
        mat4 view, proj;
    } mvp;

    struct {
        int width, height;
    } screen;


};

#endif // H_C_CAMERA
