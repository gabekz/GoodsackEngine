#ifndef H_C_CAMERA
#define H_C_CAMERA

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <entity/v1/ecs.h>

#if !(USING_GENERATED_COMPONENTS)
struct ComponentCamera
{
    vec3 axisUp, center,
      front; // TODO: Currently used for Audio Listener orientation.
    float fov;
    float speed, sensitivity;

    float lastX, lastY;
    float yaw, pitch;

    si16 firstMouse; // bool

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
#endif

void
s_camera_init(ECS *ecs);

#endif // H_C_CAMERA
