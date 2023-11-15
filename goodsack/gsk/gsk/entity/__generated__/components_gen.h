// @generated file

#ifndef H_COMPONENTS_GEN
#define H_COMPONENTS_GEN

#include <util/maths.h>
#include <util/sysdefs.h>

#include <entity/ecsdefs.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifndef CACHE_LINE
#define CACHE_LINE ECS_COMPONENTS_ALIGN_BYTES
#endif // CACHE_LINE

// typedef (void *)(ResRef)
#define ResRef void *

typedef enum ECSComponentType_t {
    C_ANIMATOR,
    C_AUDIOLISTENER,
    C_AUDIOSOURCE,
    C_CAMERA,
    C_CAMERALOOK,
    C_CAMERAMOVEMENT,
    C_LIGHT,
    C_MODEL,
    C_RENDERLAYER,
    C_RIGIDBODY,
    C_COLLIDER,
    C_TEST,
    C_TRANSFORM,
    C_WEAPON,
    C_WEAPONSWAY,
} ECSComponentType;

#define ECSCOMPONENT_LAST 14

#if ECS_COMPONENTS_PACKED
#pragma pack(push, 1)
// #else
//  pragma pack(push, ECS_COMPONENTS_ALIGN_BYTES)
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
    char filePath[256];
    ui32 looping;
};

struct ComponentCamera
{
    CACHE_ALIGN(vec3 axisUp);
    CACHE_ALIGN(vec3 center);
    CACHE_ALIGN(f32 farZ);
    CACHE_ALIGN(f32 fov);
    CACHE_ALIGN(vec3 front);
    CACHE_ALIGN(mat4 model);
    CACHE_ALIGN(f32 nearZ);
    CACHE_ALIGN(mat4 proj);
    CACHE_ALIGN(ui32 renderLayer);
    CACHE_ALIGN(si32 screenHeight);
    CACHE_ALIGN(si32 screenWidth);
    CACHE_ALIGN(ResRef uniformBuffer);
    CACHE_ALIGN(ResRef uniformBufferMapped);
    CACHE_ALIGN(ResRef uniformBufferMemory);
    CACHE_ALIGN(mat4 view);
};

struct ComponentCameraLook
{
    CACHE_ALIGN(si32 firstMouse);
    CACHE_ALIGN(f32 lastX);
    CACHE_ALIGN(f32 lastY);
    CACHE_ALIGN(f32 pitch);
    CACHE_ALIGN(f32 sensitivity);
    CACHE_ALIGN(f32 yaw);
};

struct ComponentCameraMovement
{
    CACHE_ALIGN(f32 speed);
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

struct ComponentRenderLayer
{
    ui32 renderLayer;
};

struct ComponentRigidbody
{
    vec3 gravity, velocity, force, angular_velocity;
    float mass;
    ResRef solver;
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
    CACHE_ALIGN(vec3 orientation);
    void *parent;
    CACHE_ALIGN(vec3 position);
    CACHE_ALIGN(vec3 scale);
};

struct ComponentWeapon
{
    CACHE_ALIGN(float damage);
    CACHE_ALIGN(int entity_camera);
    CACHE_ALIGN(vec3 pos_starting);
    CACHE_ALIGN(vec3 rot_starting);
};

struct ComponentWeaponSway
{
    float sway_amount;
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
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_CAMERALOOK, sizeof(struct ComponentCameraLook));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_CAMERAMOVEMENT, sizeof(struct ComponentCameraMovement));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_LIGHT, sizeof(struct ComponentLight));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_MODEL, sizeof(struct ComponentModel));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_RENDERLAYER, sizeof(struct ComponentRenderLayer));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_RIGIDBODY, sizeof(struct ComponentRigidbody));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_COLLIDER, sizeof(struct ComponentCollider));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_TEST, sizeof(struct ComponentTest));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_WEAPON, sizeof(struct ComponentWeapon));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_WEAPONSWAY, sizeof(struct ComponentWeaponSway));
}

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // COMPONENTS_GEN_IMPLEMENTATION