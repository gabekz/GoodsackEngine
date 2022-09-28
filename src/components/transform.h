#ifndef H_C_TRANSFORM
#define H_C_TRANSFORM

#include <util/maths.h>

struct ComponentTransform {
    vec3 position, scale, rotation;
    struct {
        mat4 matrix; 
    } mvp;
};

void transform_translate(struct ComponentTransform *transform, vec3 position);
void transform_rotate(struct ComponentTransform *transform, vec3 rotation);
void transform_scale(struct ComponentTransform *transform);

#endif // H_C_TRANSFORM