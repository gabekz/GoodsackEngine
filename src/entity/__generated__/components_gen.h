// @generated file

#include <util/maths.h>
#include <util/sysdefs.h>

//#ifdef __cplusplus
// extern "C" {
//#endif // __cplusplus

typedef(void *)(ResRef)

  typedef enum _ecs_component_types {
      C_COMPONENTTEST,
      C_TRANSFORM,
  } ECSComponentTypes;

typedef struct ComponentTest_t
{
    si32 movement_increment;
    f32 rotation_speed;
} ComponentTest;

typedef struct Transform_t
{
    mat4 model;
    vec3 orientation;
    vec3 position;
    vec3 scale;
} Transform;

inline void
_ecs_init_internal(ECS *ecs)
{
    ecs_component_register(ecs, C_COMPONENTTEST, sizeof(ComponentTest));
    ecs_component_register(ecs, C_TRANSFORM, sizeof(Transform));
}