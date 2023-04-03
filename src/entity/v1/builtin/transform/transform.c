#include "transform.h"

#include <core/graphics/shader/shader.h>
#include <entity/v1/ecs.h>

void
transform_translate(struct ComponentTransform *transform, vec3 position)
{
}

void
transform_position(struct ComponentTransform *transform, vec3 position)
{
    mat4 matrix = GLM_MAT4_IDENTITY_INIT;
    glm_translate(matrix, position);
    glm_vec3_copy(position, transform->position);
    glm_mat4_copy(matrix, transform->mvp.model);
}

/*
void transform_rotate(struct ComponentTransform *transform, vec3 rotation) {
    glm_rotate(*transform->mvp.matrix, rotation);
}
*/

static void
init(Entity e)
{
    if (!(ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    mat4 m4i = GLM_MAT4_IDENTITY_INIT;
    glm_translate(m4i, transform->position);
    glm_mat4_copy(m4i, transform->mvp.model);

    // TODO: stupid hack.
    float scaleCheck =
      (transform->scale[0] * transform->scale[1] * transform->scale[2]);
    if (scaleCheck <= 0) { glm_vec3_one(transform->scale); }

    // glm_mat4_copy(matrix, transform->mvp.matrix);
    // printf("position %f, %f, %f", transform->position[0],
    // transform->position[1], transform->position[2]); *transform->mvp.matrix =
    // matrix; transform->mvp.matrix = matrix; transform->mvp.matrix = matrix;

    // TODO: Make descriptor set HERE
}

static void
update(Entity e)
{
    if (!(ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);
    mat4 m4i                             = GLM_MAT4_IDENTITY_INIT;
    glm_translate(m4i, transform->position);
    glm_scale(m4i, transform->scale);
    glm_mat4_copy(m4i, transform->mvp.model);
}

void
s_transform_init(ECS *ecs)
{
    ecs_component_register(ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init    = (ECSSubscriber)init,
                          .destroy = NULL,
                          .render  = NULL,
                          .update  = (ECSSubscriber)update,
                        }));
}
