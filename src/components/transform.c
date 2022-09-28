#include "transform.h"

#include <core/ecs.h>
#include <core/shader.h>

void transform_translate(struct ComponentTransform *transform, vec3 position) {
    //glm_translate(transform->mvp.matrix, position);
}

void transform_position(struct ComponentTransform *transform, vec3 position) {
    mat4 matrix = GLM_MAT4_IDENTITY_INIT;
    glm_translate(matrix, position);
    //glm_mat4_copy(matrix, transform->mvp.matrix);
    glm_vec3_copy(position, transform->position);
}

/*
void transform_rotate(struct ComponentTransform *transform, vec3 rotation) {
    glm_rotate(*transform->mvp.matrix, rotation);
}
*/

static void init(Entity e) {
    if(!(ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    mat4 m4i = GLM_MAT4_IDENTITY_INIT;
    glm_translate(m4i, transform->position);
    glm_mat4_copy(m4i, transform->mvp.model);
    //glm_mat4_copy(matrix, transform->mvp.matrix);
    //printf("position %f, %f, %f", transform->position[0], transform->position[1], transform->position[2]);
    //*transform->mvp.matrix = matrix;
    //transform->mvp.matrix = matrix;
    //transform->mvp.matrix = matrix;
}

void s_transform_init(ECS *ecs) {
    ecs_component_register(ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
    ecs_system_register(ecs, ((ECSSystem){
        .init       = (ECSSubscriber) init,
        .destroy    = NULL,
        .render     = NULL,
        .update     = NULL,
    }));
}
