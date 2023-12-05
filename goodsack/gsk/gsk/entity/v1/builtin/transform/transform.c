/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "transform.h"

#include "core/graphics/shader/shader.h"
#include "entity/v1/ecs.h"

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
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);
    transform->hasParent                 = false; // if not already set..

    mat4 m4i = GLM_MAT4_IDENTITY_INIT;

    // Get parent transform (if exists)
    if (transform->parent) {
        transform->hasParent = true;
        struct ComponentTransform *parentTransform =
          gsk_ecs_get(*(gsk_Entity *)transform->parent, C_TRANSFORM);
        glm_mat4_copy(transform->parent, m4i);
    }

    glm_translate(m4i, transform->position);
    glm_mat4_copy(m4i, transform->model);

    // stupid hack which basically doesn't allow a zero scale.
    if (!transform->scale[0] && !transform->scale[1] && !transform->scale[2]) {
        glm_vec3_one(transform->scale);
    }

    // TODO: [vulkan] Make descriptor set HERE
}

static void
late_update(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    mat4 m4i = GLM_MAT4_IDENTITY_INIT;

    if (gsk_ecs_has(e, C_CAMERA)) {
        struct ComponentCamera *camera = gsk_ecs_get(e, C_CAMERA);
        glm_mat4_inv(camera->view, transform->model);
        return;
    }

    if (transform->hasParent) {
        struct ComponentTransform *parent =
          gsk_ecs_get(*(gsk_Entity *)transform->parent, C_TRANSFORM);
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
s_transform_init(gsk_ECS *ecs)
{
    //_ECS_DECL_COMPONENT(ecs, C_TRANSFORM, sizeof(struct ComponentTransform));
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init        = (gsk_ECSSubscriber)init,
                              .destroy     = NULL,
                              .render      = NULL,
                              .update      = NULL,
                              .late_update = (gsk_ECSSubscriber)late_update,
                            }));
}
