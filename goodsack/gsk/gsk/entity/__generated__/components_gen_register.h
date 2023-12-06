#ifndef H_COMPONENTS_GEN_REGISTER
#define H_COMPONENTS_GEN_REGISTER

#include <entity/__generated__/components_gen.h>
#include <entity/ecs.h>

static inline void
_ecs_init_internal_gen(gsk_ECS *ecs)
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
      ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
}

#endif // H_COMPONENTS_GEN_REGISTER