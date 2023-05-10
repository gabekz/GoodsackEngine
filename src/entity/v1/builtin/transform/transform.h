#ifndef H_C_TRANSFORM
#define H_C_TRANSFORM

#include <entity/v1/ecs.h>
#include <util/maths.h>

#if !(USING_GENERATED_COMPONENTS)
struct ComponentTransform
{
    vec3 position, orientation, scale;
    struct
    {
        mat4 model;
    } mvp;
    float test;
};
#endif

void
transform_position(struct ComponentTransform *transform, vec3 position);
void
transform_translate(struct ComponentTransform *transform, vec3 position);
void
transform_rotate(struct ComponentTransform *transform, vec3 rotation);
void
transform_scale(struct ComponentTransform *transform);

void
s_transform_init(ECS *ecs);

#endif // H_C_TRANSFORM
