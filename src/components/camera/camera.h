#ifndef H_C_CAMERA
#define H_C_CAMERA

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <ecs/ecs.h>

struct ComponentCamera {
    vec3 position, axisUp;
    float fov;
    float speed, sensitivity;
    ui32 uboId;
    struct {
        float nearZ, farZ;
    } clipping;
    struct {
        mat4 view, proj;
    } mvp;
    struct {
        int width, height;
    } screen;
};

void s_camera_init(ECS *ecs);

#endif // H_C_CAMERA
