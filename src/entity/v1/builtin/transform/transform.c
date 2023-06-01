#include "transform.h"

#include <core/graphics/shader/shader.h>
#include <entity/v1/ecs.h>

// #define ECS_SYSTEM
//  ECS_SYSTEM_DECLARE()

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
    glm_mat4_copy(matrix, transform->model);
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
    transform->hasParent                 = false; // if not already set..

    mat4 m4i = GLM_MAT4_IDENTITY_INIT;

    // Get parent transform (if exists)
    if (transform->parent) {
        transform->hasParent = true;
        struct ComponentTransform *parentTransform =
          ecs_get(*(Entity *)transform->parent, C_TRANSFORM);
        glm_mat4_copy(transform->parent, m4i);
    }

    glm_translate(m4i, transform->position);
    glm_mat4_copy(m4i, transform->model);

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
late_update(Entity e)
{
    if (!(ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = ecs_get(e, C_TRANSFORM);

    mat4 m4i = GLM_MAT4_IDENTITY_INIT;

    if (ecs_has(e, C_CAMERA)) {
        struct ComponentCamera *camera = ecs_get(e, C_CAMERA);
        glm_mat4_inv(camera->view, transform->model);
        return;
    }

    if (transform->hasParent) {
        struct ComponentTransform *parent =
          ecs_get(*(Entity *)transform->parent, C_TRANSFORM);
        glm_mat4_copy(parent->model, m4i);
    }

    glm_translate(m4i, transform->position);

    glm_rotate_x(m4i, glm_rad(transform->orientation[0]), m4i);
    glm_rotate_y(m4i, glm_rad(transform->orientation[1]), m4i);
    glm_rotate_z(m4i, glm_rad(transform->orientation[2]), m4i);

    glm_scale(m4i, transform->scale);

    glm_mat4_copy(m4i, transform->model);
}

void
s_transform_init(ECS *ecs)
{
    //_ECS_DECL_COMPONENT(ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
    ecs_system_register(ecs,
                        ((ECSSystem) {
                          .init        = (ECSSubscriber)init,
                          .destroy     = NULL,
                          .render      = NULL,
                          .update      = NULL,
                          .late_update = (ECSSubscriber)late_update,
                        }));
}
