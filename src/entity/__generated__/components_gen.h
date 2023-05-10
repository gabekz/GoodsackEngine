// @generated file

#include <util/maths.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define ResRef void *

#define ECSCOMPONENT_LAST C_TRANSFORM
typedef enum _ecs_component_types {
    C_ANIMATOR = 0,
    C_AUDIOLISTENER,
    C_AUDIOSOURCE,
    C_CAMERA,
    C_LIGHT,
    C_MODEL,
    C_TEST,
    C_TRANSFORM,
} ECSComponentType;

struct ComponentAnimator
{
    ResRef cntAnimation;
    si32 cntKeyframeIndex;
    si32 cntTime;
    si32 nxtKeyframeIndex;
    f32 timerNow;
    f32 timerStart;
};

struct ComponentAudioListener
{
    si32 a;
};

struct ComponentAudioSource
{
    ui32 bufferId;
    int looping;
};

struct ComponentCamera
{
    vec3 axisUp;
    vec3 center;
    f32 farZ;
    si32 firstMouse;
    f32 fov;
    vec3 front;
    f32 lastX;
    f32 lastY;
    mat4 model;
    f32 nearZ;
    f32 pitch;
    mat4 proj;
    si32 screenHeight;
    si32 screenWidth;
    f32 sensitivity;
    f32 speed;
    ui32 uboId;
    ResRef uniformBuffer;
    ResRef uniformBufferMapped;
    ResRef uniformBufferMemory;
    mat4 view;
    f32 yaw;
};

struct ComponentLight
{
    int color;
    ui32 type;
};

struct ComponentModel
{
    ResRef material;
    ResRef mesh;
    ResRef pModel;
    ui32 vbo;
};

struct ComponentTest
{
    si32 movement_increment;
    f32 rotation_speed;
};

struct ComponentTransform
{
    mat4 model;
    vec3 orientation;
    vec3 position;
    vec3 scale;
};

/*
static inline void
_ecs_init_internal(ECS *ecs)
{
    ecs_component_register(ecs, C_ANIMATOR, sizeof(struct ComponentAnimator));
    ecs_component_register(
      ecs, C_AUDIOLISTENER, sizeof(struct ComponentAudioListener));
    ecs_component_register(
      ecs, C_AUDIOSOURCE, sizeof(struct ComponentAudioSource));
    ecs_component_register(ecs, C_CAMERA, sizeof(struct ComponentCamera));
    ecs_component_register(ecs, C_LIGHT, sizeof(struct ComponentLight));
    ecs_component_register(ecs, C_MODEL, sizeof(struct ComponentModel));
    ecs_component_register(ecs, C_TEST, sizeof(struct ComponentTest));
    ecs_component_register(ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
}
*/

#ifdef __cplusplus
}
#endif // __cplusplus