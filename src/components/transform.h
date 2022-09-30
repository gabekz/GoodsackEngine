#ifndef H_C_TRANSFORM
#define H_C_TRANSFORM

#include <util/maths.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <core/ecs.h>

struct ComponentTransform {
    vec3 position, scale;
    struct {
        mat4 model; 
    } mvp;
    float test;
};

void transform_translate(struct ComponentTransform *transform, vec3 position);
void transform_rotate(struct ComponentTransform *transform, vec3 rotation);
void transform_scale(struct ComponentTransform *transform);

void s_transform_init(ECS *ecs);

#ifdef __cplusplus
}
#endif

#endif // H_C_TRANSFORM
