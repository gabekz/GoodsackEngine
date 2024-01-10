/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Move solvers to separate file

#include "player_controller-system.h"

#include "util/logger.h"
#include "util/sysdefs.h"

static void
init(gsk_Entity entity)
{
    if (!gsk_ecs_has(entity, C_PLAYER_CONTROLLER)) { return; }

    if (!gsk_ecs_has(entity, C_RIGIDBODY)) {
        LOG_ERROR("Player Controller is kind of useless without a rigidbody..");
        return;
    }
}

static void
update(gsk_Entity entity)
{
    if (!gsk_ecs_has(entity, C_PLAYER_CONTROLLER)) { return; }
    struct ComponentPlayerController *cmp_controller =
      gsk_ecs_get(entity, C_PLAYER_CONTROLLER);

    struct ComponentRigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    struct ComponentCollider *cmp_collider   = gsk_ecs_get(entity, C_COLLIDER);

    // camera-child
    gsk_Entity ent_camera =
      (gsk_Entity) {.index = cmp_controller->entity_camera, .ecs = entity.ecs};

    if (!gsk_ecs_has(ent_camera, C_CAMERA)) {
        LOG_WARN("camera-child does not have camera a Camera component!");
    }
    struct ComponentCamera *cmp_camera = gsk_ecs_get(ent_camera, C_CAMERA);

    // --------- logic ---------

    u8 is_moving, is_grounded = FALSE;

    is_grounded = (cmp_collider->isColliding);

    vec3 direction, cross, newvel = GLM_VEC3_ZERO_INIT;
    direction[0] = cmp_camera->front[0]; // copy x-axis
    direction[2] = cmp_camera->front[2]; // copy z-axis

    glm_vec3_scale(direction, cmp_controller->speed, direction);

    direction[1] = cmp_rigidbody->linear_velocity[1]; // keep y-axis

    // no mid-air movement
    if (!is_grounded) return;

    GLFWwindow *window = entity.ecs->renderer->window;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_add(newvel, direction, newvel);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_sub(newvel, direction, newvel);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_vec3_crossn(direction, cmp_camera->axisUp, cross);
        glm_vec3_scale(cross, cmp_controller->speed, cross);
        glm_vec3_sub(newvel, cross, newvel);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_vec3_crossn(direction, cmp_camera->axisUp, cross);
        glm_vec3_scale(cross, cmp_controller->speed, cross);
        glm_vec3_add(newvel, cross, newvel);
    }

    // TODO: Move to fixed_update
    if (is_grounded && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cmp_rigidbody->linear_velocity[1] = newvel[1] + 5;
    }

    is_moving = (glm_vec3_norm(newvel) > 0);

    // directly modify velocity
    if (is_moving) {
        cmp_rigidbody->linear_velocity[0] = newvel[0];
        cmp_rigidbody->linear_velocity[2] = newvel[2];
        // glm_vec3_copy(newvel, cmp_rigidbody->linear_velocity);
    }
}

//-----------------------------------------------------------------------------
void
s_player_controller_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init   = (gsk_ECSSubscriber)init,
                              .update = (gsk_ECSSubscriber)update,
                            }));
}
