// @generated file

#ifndef H_COMPONENTS_GEN
#define H_COMPONENTS_GEN

#include <util/maths.h>
#include <util/sysdefs.h>

#include <entity/ecsdefs.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// typedef (void *)(ResRef)
#define ResRef void *

typedef enum ECSComponentType_t {
    C_ANIMATOR,
    C_AUDIOLISTENER,
    C_AUDIOSOURCE,
    C_CAMERA,
    C_LIGHT,
    C_MODEL,
    C_RIGIDBODY,
    C_COLLIDER,
    C_TEST,
    C_TRANSFORM,
    C_WEAPON,
} ECSComponentType;

#define ECSCOMPONENT_LAST 10

#if ECS_COMPONENTS_PACKED
#pragma pack(push, 1)
//#else
//pragma pack(push, ECS_COMPONENTS_ALIGN_BYTES)
#endif // ECS_COMPONENTS_PACKED

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
    const char *filePath;
    ui32 looping;
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
    vec4 color;
    ui32 type;
};

struct ComponentModel
{
    ResRef material;
    ResRef mesh;
    char modelPath[256];
    ResRef pModel;
    ui32 vbo;
    ResRef vkVBO;
    struct
    {
        ui16 renderMode : 1;
        ui16 drawMode : 2;
        ui16 cullMode : 3;
    } properties;
};

struct ComponentRigidbody
{
    vec3 gravity, velocity, force;
    float mass;
};

struct ComponentCollider
{
    int type;
    void *pCollider;
    ui32 isColliding;
};

struct ComponentTest
{
    si32 movement_increment;
    f32 rotation_speed;
};

struct ComponentTransform
{
    ui16 hasParent;
    mat4 model;
    vec3 orientation;
    void *parent;
    vec3 position;
    vec3 scale;
};

struct ComponentWeapon
{
    float damage;
    vec3 pos_starting;
    vec3 rot_starting;
};

#if ECS_COMPONENTS_PACKED
#pragma pack(pop)
#endif // ECS_COMPONENTS_PACKED

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // H_COMPONENTS_GEN

#ifdef COMPONENTS_GEN_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

static inline void
_ecs_init_internal_gen(ECS *ecs)
{
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_ANIMATOR, sizeof(struct ComponentAnimator));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_AUDIOLISTENER, sizeof(struct ComponentAudioListener));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_AUDIOSOURCE, sizeof(struct ComponentAudioSource));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_CAMERA, sizeof(struct ComponentCamera));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_LIGHT, sizeof(struct ComponentLight));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_MODEL, sizeof(struct ComponentModel));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_RIGIDBODY, sizeof(struct ComponentRigidbody));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_COLLIDER, sizeof(struct ComponentCollider));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_TEST, sizeof(struct ComponentTest));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_WEAPON, sizeof(struct ComponentWeapon));
}

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // COMPONENTS_GEN_IMPLEMENTATION
