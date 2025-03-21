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
    C_BANE,
    C_BONE_ATTACHMENT,
    C_CAMERA,
    C_CAMERALOOK,     // TODO: Remove (new package)
    C_CAMERAMOVEMENT, // TODO: Remove (new package)
    C_COLLIDER,
    C_ENEMY, // TODO: Remove (testing)
    C_ENTITY_REFERENCE,
    C_FLAMMABLE, // TODO: Remove (new package)
    C_HEALTH,    // TODO: Remove (new package)
    C_LIGHT,
    C_MODEL,
    C_PARTICLE_EMITTER,
    C_PLAYER_CONTROLLER,  // TODO: Remove (new package)
    C_PROJECTILE_SPAWNER, // TODO: Remove (new package)
    C_RENDERLAYER,
    C_RIGIDBODY,
    C_SWORD_CONTROLLER, // TODO: Remove (testing)
    C_TRANSFORM,
    C_WEAPON,     // TODO: Remove (new package)
    C_WEAPONSWAY, // TODO: Remove (new package)
} ECSComponentType;

#define ECSCOMPONENT_LAST C_WEAPONSWAY

#if ECS_COMPONENTS_PACKED
#pragma pack(push, 1)
// #else
//  pragma pack(push, ECS_COMPONENTS_ALIGN_BYTES)
#endif // ECS_COMPONENTS_PACKED

struct ComponentAnimator
{
    ResRef cntAnimation;
    s32 cntKeyframeIndex;
    s32 cntTime;
    s32 nxtKeyframeIndex;
    f32 timerNow;
    f32 timerStart;

    u16 is_transition_delayed;
    u16 is_looping;
    u16 is_playing;

    u16 force_replay;
};

struct ComponentAudioListener
{
    s32 a;
};

struct ComponentAudioSource
{
    CACHE_ALIGN(ResRef audio_clip);
    CACHE_ALIGN(u32 buffer_audio);
    CACHE_ALIGN(u32 buffer_source);
    CACHE_ALIGN(char filePath[256]);
    CACHE_ALIGN(u8 is_looping);
    CACHE_ALIGN(u8 is_playing);
    CACHE_ALIGN(f32 max_distance);
    CACHE_ALIGN(f32 min_distance);
    CACHE_ALIGN(u8 play_on_start);
};

typedef struct ComponentBane
{
    CACHE_ALIGN(s32 damage);
} gsk_C_Bane;

typedef struct ComponentBoneAttachment
{
    CACHE_ALIGN(u32 bone_id);
    CACHE_ALIGN(int entity_skeleton);
    CACHE_ALIGN(ResRef p_joint);
} gsk_C_BoneAttachment;

typedef struct ComponentCamera
{
    CACHE_ALIGN(vec3 axisUp);
    CACHE_ALIGN(vec3 center);
    CACHE_ALIGN(f32 farZ);
    CACHE_ALIGN(f32 fov);
    CACHE_ALIGN(vec3 front);
    CACHE_ALIGN(mat4 model);
    CACHE_ALIGN(f32 nearZ);
    CACHE_ALIGN(mat4 proj);
    CACHE_ALIGN(u32 renderLayer);
    CACHE_ALIGN(s32 screenHeight);
    CACHE_ALIGN(s32 screenWidth);
    CACHE_ALIGN(f32 shake_amount);
    CACHE_ALIGN(ResRef uniformBuffer);
    CACHE_ALIGN(ResRef uniformBufferMapped);
    CACHE_ALIGN(ResRef uniformBufferMemory);
    CACHE_ALIGN(mat4 view);
} gsk_C_Camera;

typedef struct ComponentCameraLook
{
    CACHE_ALIGN(s32 firstMouse);
    CACHE_ALIGN(f32 lastX);
    CACHE_ALIGN(f32 lastY);
    CACHE_ALIGN(f32 pitch);
    CACHE_ALIGN(f32 sensitivity);
    CACHE_ALIGN(f32 yaw);
} gsk_C_CameraLook;

typedef struct ComponentCameraMovement
{
    CACHE_ALIGN(f32 speed);
} gsk_C_CameraMovement;

struct ComponentCollider
{
    CACHE_ALIGN(s32 type);
    CACHE_ALIGN(void *pCollider);
    CACHE_ALIGN(u32 isColliding);
    CACHE_ALIGN(void *p_mesh);
    CACHE_ALIGN(vec3 box_bounds_min);
    CACHE_ALIGN(vec3 box_bounds_max);
};

struct ComponentEnemy
{
    CACHE_ALIGN(int enemy_type);
    CACHE_ALIGN(vec3 hit_position);
    CACHE_ALIGN(u8 is_wep_loaded);
};

typedef struct ComponentEntityReference
{
    CACHE_ALIGN(int entity_ref_id);
} gsk_C_EntityReference;

struct ComponentFlammable
{
    CACHE_ALIGN(f32 burn_speed);
    CACHE_ALIGN(f32 cooldown_speed);
    CACHE_ALIGN(f32 current_heat);
    CACHE_ALIGN(f32 ignition_point);
    CACHE_ALIGN(u8 is_burning);
    CACHE_ALIGN(f32 max_heat);
};

struct ComponentHealth
{
    CACHE_ALIGN(int current_health);
    CACHE_ALIGN(int last_health);
    CACHE_ALIGN(int max_health);
    CACHE_ALIGN(u32 is_alive);
    CACHE_ALIGN(u32 event_health_change);
};

struct ComponentLight
{
    vec4 color;
    u32 type;
};

struct ComponentModel
{
    ResRef material;
    ResRef mesh;
    char modelPath[256];
    ResRef pModel;
    u32 vbo;
    ResRef vkVBO;
    struct
    {
        u16 renderMode : 1;
        u16 drawMode : 2;
        u16 cullMode : 3;
    } properties;

    u8 cast_shadows; // TODO:
};

struct ComponentParticleEmitter
{
    CACHE_ALIGN(void *p_particle_system);
    CACHE_ALIGN(f32 speed);
    CACHE_ALIGN(u8 is_awake);
    CACHE_ALIGN(void *p_meshdata);
};

struct ComponentPlayerController
{
    CACHE_ALIGN(f32 speed);
    CACHE_ALIGN(f32 sprint_timer);
    CACHE_ALIGN(int entity_camera);
    CACHE_ALIGN(int walk_direction);
    CACHE_ALIGN(vec2 move_axes);
    CACHE_ALIGN(u8 can_jump);
    CACHE_ALIGN(u8 is_grounded);
    CACHE_ALIGN(u8 is_jumping);
    CACHE_ALIGN(f32 jump_force);
};
struct ComponentProjectileSpawner
{
    CACHE_ALIGN(ResRef projectile_material);
    CACHE_ALIGN(ResRef projectile_model);
    CACHE_ALIGN(ResRef projectile_sound);
};

struct ComponentRenderLayer
{
    u32 renderLayer;
};

struct ComponentRigidbody
{
    vec3 angular_velocity, linear_velocity, gravity;
    vec3 force_impulse, force_velocity, torque;
    float mass, static_friction, dynamic_friction;
    float inverse_mass, inverse_inertia;
    ResRef solver;
    u8 is_kinematic;
    u8 disable_rotation;
};

struct ComponentSwordController
{
    CACHE_ALIGN(int entity_camera);
    CACHE_ALIGN(int entity_flock);
    CACHE_ALIGN(int weapon_state);
    CACHE_ALIGN(int last_direction);
    CACHE_ALIGN(float state_timer);
    CACHE_ALIGN(float charge_time);
};

typedef struct ComponentTransform
{
    CACHE_ALIGN(vec3 forward);
    CACHE_ALIGN(u16 hasParent);
    CACHE_ALIGN(mat4 model);
    CACHE_ALIGN(vec3 orientation);
    CACHE_ALIGN(void *parent);
    CACHE_ALIGN(vec3 position);
    CACHE_ALIGN(vec3 world_position);
    CACHE_ALIGN(vec3 scale);
} gsk_C_Transform;

struct ComponentWeapon
{
    CACHE_ALIGN(float damage);
    CACHE_ALIGN(int entity_camera);
    CACHE_ALIGN(vec3 pos_starting);
    CACHE_ALIGN(vec3 rot_starting);
};

struct ComponentWeaponSway
{
    CACHE_ALIGN(float move_step_time);
    CACHE_ALIGN(float sway_amount);
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
_ecs_init_internal_gen(gsk_ECS *ecs)
{
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_ANIMATOR, sizeof(struct ComponentAnimator));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_AUDIOLISTENER, sizeof(struct ComponentAudioListener));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_AUDIOSOURCE, sizeof(struct ComponentAudioSource));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_BANE, sizeof(struct ComponentBane));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_BONE_ATTACHMENT, sizeof(struct ComponentBoneAttachment));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_CAMERA, sizeof(struct ComponentCamera));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_CAMERALOOK, sizeof(struct ComponentCameraLook));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_CAMERAMOVEMENT, sizeof(struct ComponentCameraMovement));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_COLLIDER, sizeof(struct ComponentCollider));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_ENEMY, sizeof(struct ComponentEnemy));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_ENTITY_REFERENCE, sizeof(struct ComponentEntityReference));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_FLAMMABLE, sizeof(struct ComponentFlammable));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_HEALTH, sizeof(struct ComponentHealth));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_LIGHT, sizeof(struct ComponentLight));
    _ECS_DECL_COMPONENT_INTERN(ecs, C_MODEL, sizeof(struct ComponentModel));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_PARTICLE_EMITTER, sizeof(struct ComponentParticleEmitter));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_PLAYER_CONTROLLER, sizeof(struct ComponentPlayerController));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_PROJECTILE_SPAWNER, sizeof(struct ComponentProjectileSpawner));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_RENDERLAYER, sizeof(struct ComponentRenderLayer));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_RIGIDBODY, sizeof(struct ComponentRigidbody));
    _ECS_DECL_COMPONENT_INTERN(
      ecs, C_SWORD_CONTROLLER, sizeof(struct ComponentSwordController));
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
