#ifndef H_C_TRANSFORM
#define H_C_TRANSFORM

#include <ecs/ecs.h>
#include <util/maths.h>

struct ComponentTransform
{
    vec3 position, orientation, scale;
    struct
    {
        mat4 model;
    } mvp;
    float test;
};

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
