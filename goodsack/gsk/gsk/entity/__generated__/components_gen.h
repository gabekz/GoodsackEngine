// @generated file

#ifndef H_COMPONENTS_GEN
#define H_COMPONENTS_GEN

#include <util/maths.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// typedef (void *)(ResRef)
#define ResRef void *

typedef enum ECSComponentType_t {
    C_ANIMATOR,
    C_AUDIOLISTENER,
    C_AUDIOSOURCE,
    C_BANE,
    C_BONE_ATTACHMENT,
    C_CAMERA,
    C_CAMERALOOK,
    C_CAMERAMOVEMENT,
    C_COLLIDER,
    C_ENEMY,
    C_ENTITY_REFERENCE,
    C_FLAMMABLE,
    C_HEALTH,
    C_LIGHT,
    C_MODEL,
    C_PARTICLE_EMITTER,
    C_PLAYER_CONTROLLER,
    C_PROJECTILE_SPAWNER,
    C_RENDERLAYER,
    C_RIGIDBODY,
    C_SWORD_CONTROLLER,
    C_TRANSFORM,
    C_WEAPON,
    C_WEAPONSWAY,
} ECSComponentType;

#define ECSCOMPONENT_LAST 23

struct ComponentAnimator
{
    CACHE_ALIGN(u32 animation_index);
    CACHE_ALIGN(ResRef cntAnimation);
    CACHE_ALIGN(s32 cntKeyframeIndex);
    CACHE_ALIGN(s32 cntTime);
    CACHE_ALIGN(u16 force_replay);
    CACHE_ALIGN(u16 is_holding_end_frame);
    CACHE_ALIGN(u16 is_looping);
    CACHE_ALIGN(u16 is_playing);
    CACHE_ALIGN(u16 is_transition_delayed);
    CACHE_ALIGN(s32 nxtKeyframeIndex);
    CACHE_ALIGN(ResRef p_animation_set);
    CACHE_ALIGN(f32 timerNow);
    CACHE_ALIGN(f32 timerStart);
};

struct ComponentAudioListener
{
    CACHE_ALIGN(s32 a);
};

struct ComponentAudioSource
{
    CACHE_ALIGN(ResRef audio_clip);
    CACHE_ALIGN(u32 buffer_audio);
    CACHE_ALIGN(u32 buffer_source);
    CACHE_ALIGN(const char *filePath);
    CACHE_ALIGN(u16 is_looping);
    CACHE_ALIGN(u16 is_playing);
    CACHE_ALIGN(f32 min_distance, max_distance);
    CACHE_ALIGN(f32 pitch);
    CACHE_ALIGN(u16 play_on_start);
};

struct ComponentBane
{
    CACHE_ALIGN(s32 damage);
    CACHE_ALIGN(u16 is_instant_kill);
};

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

struct ComponentCameraMovement
{
    CACHE_ALIGN(f32 speed);
};

struct ComponentCollider
{
    CACHE_ALIGN(vec3 box_bounds_max);
    CACHE_ALIGN(vec3 box_bounds_min);
    CACHE_ALIGN(u16 isColliding);
    CACHE_ALIGN(u16 is_trigger);
    CACHE_ALIGN(ResRef pCollider);
    CACHE_ALIGN(ResRef p_mesh);
    CACHE_ALIGN(f32 radius);
    CACHE_ALIGN(s32 type);
};

struct ComponentEnemy
{
    CACHE_ALIGN(s32 enemy_type);
    CACHE_ALIGN(vec3 hit_position);
    CACHE_ALIGN(u16 is_wep_loaded);
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
    CACHE_ALIGN(int entity_emitter_id);
    CACHE_ALIGN(f32 ignition_point);
    CACHE_ALIGN(u16 is_burning);
    CACHE_ALIGN(f32 max_heat);
};

struct ComponentHealth
{
    CACHE_ALIGN(s32 current_health);
    CACHE_ALIGN(u16 event_health_change);
    CACHE_ALIGN(u16 is_alive);
    CACHE_ALIGN(s32 last_health);
    CACHE_ALIGN(s32 max_health);
};

struct ComponentLight
{
    CACHE_ALIGN(vec4 color);
    CACHE_ALIGN(f32 intensity);
    CACHE_ALIGN(u32 light_index);
    CACHE_ALIGN(u32 type);
};

struct ComponentModel
{
    CACHE_ALIGN(ResRef _skeleton);
    CACHE_ALIGN(u16 cast_shadows);
    CACHE_ALIGN(ResRef material);
    CACHE_ALIGN(ResRef mesh);
    CACHE_ALIGN(const char *modelPath);
    CACHE_ALIGN(ResRef pModel);
    CACHE_ALIGN(u32 vbo);
    CACHE_ALIGN(ResRef vkVBO);
};

struct ComponentParticleEmitter
{
    CACHE_ALIGN(u16 is_awake);
    CACHE_ALIGN(ResRef p_meshdata);
    CACHE_ALIGN(ResRef p_particle_system);
    CACHE_ALIGN(ResRef p_settings);
    CACHE_ALIGN(f32 speed);
};

struct ComponentPlayerController
{
    CACHE_ALIGN(u16 can_jump);
    CACHE_ALIGN(int entity_camera);
    CACHE_ALIGN(u16 is_grounded);
    CACHE_ALIGN(u16 is_jumping);
    CACHE_ALIGN(f32 jump_force);
    CACHE_ALIGN(vec2 move_axes);
    CACHE_ALIGN(f32 speed);
    CACHE_ALIGN(f32 sprint_timer);
    CACHE_ALIGN(s32 walk_direction);
};

struct ComponentProjectileSpawner
{
    CACHE_ALIGN(ResRef projectile_material);
    CACHE_ALIGN(ResRef projectile_model);
    CACHE_ALIGN(ResRef projectile_sound);
};

struct ComponentRenderLayer
{
    CACHE_ALIGN(u32 renderLayer);
};

struct ComponentRigidbody
{
    CACHE_ALIGN(vec3 angular_velocity);
    CACHE_ALIGN(u16 disable_rotation);
    CACHE_ALIGN(f32 dynamic_friction);
    CACHE_ALIGN(vec3 force_impulse);
    CACHE_ALIGN(vec3 force_velocity);
    CACHE_ALIGN(vec3 gravity);
    CACHE_ALIGN(f32 inverse_inertia);
    CACHE_ALIGN(f32 inverse_mass);
    CACHE_ALIGN(u16 is_kinematic);
    CACHE_ALIGN(vec3 linear_velocity);
    CACHE_ALIGN(f32 mass);
    CACHE_ALIGN(ResRef solver);
    CACHE_ALIGN(f32 static_friction);
    CACHE_ALIGN(vec3 torque);
};

struct ComponentSwordController
{
    CACHE_ALIGN(f32 charge_time);
    CACHE_ALIGN(int entity_camera);
    CACHE_ALIGN(int entity_flock);
    CACHE_ALIGN(s32 last_direction);
    CACHE_ALIGN(f32 state_timer);
    CACHE_ALIGN(s32 weapon_state);
};

typedef struct ComponentTransform
{
    CACHE_ALIGN(vec3 forward);
    CACHE_ALIGN(u16 has_parent);
    CACHE_ALIGN(mat4 model);
    CACHE_ALIGN(vec3 orientation);
    CACHE_ALIGN(int parent_entity_id);
    CACHE_ALIGN(vec3 position);
    CACHE_ALIGN(vec3 scale);
    CACHE_ALIGN(vec3 world_position);
} gsk_C_Transform;

struct ComponentWeapon
{
    CACHE_ALIGN(f32 damage);
    CACHE_ALIGN(int entity_camera);
    CACHE_ALIGN(vec3 pos_starting);
    CACHE_ALIGN(vec3 rot_starting);
};

struct ComponentWeaponSway
{
    CACHE_ALIGN(f32 move_step_time);
    CACHE_ALIGN(f32 sway_amount);
};

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