#ifndef H_C_CAMERA
#define H_C_CAMERA

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <ecs/ecs.h>

struct ComponentCamera
{
    vec3 position, axisUp;
    float fov;
    float speed, sensitivity;

    // OpenGL
    ui32 uboId;
    //

    // Vulkan
    VkBuffer *uniformBuffer;
    VkDeviceMemory *uniformBufferMemory;
    void **uniformBufferMapped;
    //

    struct
    {
        float nearZ, farZ;
    } clipping;
    struct
    {
        int width, height;
    } screen;

    struct
    {
        // vec3 position; -- TODO: Should be part of UNIFORM structure
        mat4 model, view, proj;
    } uniform;
};

void
s_camera_init(ECS *ecs);

#endif // H_C_CAMERA
