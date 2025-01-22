/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Move input checks to update()

#include "player_controller-system.h"
#include "gsk/entity/modules/physics/mod_physics.h"
#include "player.h"

#include "util/logger.h"
#include "util/sysdefs.h"

#define JUMP_FORCE 160

static void
init(gsk_Entity entity)
{
    if (!gsk_ecs_has(entity, C_PLAYER_CONTROLLER)) { return; }

    if (!gsk_ecs_has(entity, C_RIGIDBODY))
    {
        LOG_ERROR("Player Controller is kind of useless without a rigidbody..");
        return;
    }

    struct ComponentPlayerController *cmp_controller =
      gsk_ecs_get(entity, C_PLAYER_CONTROLLER);

    cmp_controller->is_grounded = FALSE;
    cmp_controller->is_jumping  = FALSE;
    cmp_controller->can_jump    = TRUE;
    cmp_controller->jump_force  = JUMP_FORCE;
}

static void
update(gsk_Entity entity)
{
    if (!gsk_ecs_has(entity, C_PLAYER_CONTROLLER)) { return; }

    struct ComponentPlayerController *cmp_controller =
      gsk_ecs_get(entity, C_PLAYER_CONTROLLER);

    // store *window for input
    GLFWwindow *window = entity.ecs->renderer->window;

    // 1. zero-out walk direction, 2. check for input
    cmp_controller->walk_direction = 0;

    int input_up    = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    int input_down  = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    int input_left  = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    int input_right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    int input_jump = glfwGetKey(window, GLFW_KEY_SPACE);

    // jump begin
    if (input_jump == GLFW_PRESS && cmp_controller->can_jump)
    {
        cmp_controller->is_jumping =
          (cmp_controller->is_grounded && input_jump);
        cmp_controller->can_jump = FALSE;
    }
    // jump release
    else if (input_jump == GLFW_RELEASE && cmp_controller->is_grounded)
    {
        cmp_controller->can_jump   = TRUE;
        cmp_controller->is_jumping = FALSE;
    }

    if (cmp_controller->is_jumping) { return; }

    if (!(input_up && input_down))
    {
        if (input_up)
        {
            cmp_controller->walk_direction |= WALK_FORWARD;
        } else if (input_down)
        {
            cmp_controller->walk_direction |= WALK_BACKWARD;
        }
    }
    if (!(input_left && input_right))
    {
        if (input_left)
        {
            cmp_controller->walk_direction |= WALK_LEFT;
        } else if (input_right)
        {
            cmp_controller->walk_direction |= WALK_RIGHT;
        }
    }
}

static void
fixed_update(gsk_Entity entity)
{
    if (!gsk_ecs_has(entity, C_PLAYER_CONTROLLER)) { return; }
    if (!gsk_ecs_has(entity, C_TRANSFORM)) { return; }
    if (!gsk_ecs_has(entity, C_RIGIDBODY)) { return; }
    if (!gsk_ecs_has(entity, C_COLLIDER)) { return; }

    struct ComponentPlayerController *cmp_controller =
      gsk_ecs_get(entity, C_PLAYER_CONTROLLER);

    struct ComponentTransform *cmp_transform = gsk_ecs_get(entity, C_TRANSFORM);

    struct ComponentRigidbody *cmp_rigidbody = gsk_ecs_get(entity, C_RIGIDBODY);
    struct ComponentCollider *cmp_collider   = gsk_ecs_get(entity, C_COLLIDER);

    // camera-child
    gsk_Entity ent_camera =
      gsk_ecs_ent(entity.ecs, cmp_controller->entity_camera);

    if (!gsk_ecs_has(ent_camera, C_CAMERA))
    {
        LOG_ERROR("camera-child does not have camera a Camera component!");
    }
    struct ComponentCamera *cmp_camera = gsk_ecs_get(ent_camera, C_CAMERA);

    // --------- ground-check ---------

    gsk_Raycast raycast;
    vec3 dir = {0, -1.0f, 0};
    glm_vec3_copy(cmp_transform->position, raycast.origin);
    glm_vec3_copy(dir, raycast.direction);

    gsk_mod_RaycastResult result =
      gsk_mod_physics_raycast(entity, &raycast, 1.65f);

    cmp_controller->is_grounded = result.has_collision;
    // cmp_controller->is_jumping  = (cmp_controller->is_grounded &&
    // input_jump);

    // --------- logic ---------

    u8 is_moving = FALSE;

    vec3 direction, cross, newvel = GLM_VEC3_ZERO_INIT;
    direction[0] = cmp_camera->front[0]; // copy x-axis
    direction[2] = cmp_camera->front[2]; // copy z-axis

    // no mid-air movement
    if (!cmp_controller->is_grounded && !cmp_collider->isColliding) return;

    f32 speed = cmp_controller->speed;
#if 0
    if (cmp_controller->walk_direction & (WALK_FORWARD | WALK_BACKWARD)) {
        if (cmp_controller->walk_direction & (WALK_LEFT | WALK_RIGHT)) {
            speed = speed / 2;
        }
    }
    LOG_INFO("%f speed: ", speed);
#endif

    // scale speed based on direction
    glm_vec3_scale(direction, speed, direction);
    direction[1] = cmp_rigidbody->linear_velocity[1]; // keep y-axis

    if (cmp_controller->walk_direction & WALK_FORWARD)
    {
        glm_vec3_add(newvel, direction, newvel);
    }
    if (cmp_controller->walk_direction & WALK_BACKWARD)
    {
        glm_vec3_sub(newvel, direction, newvel);
    }

    if (cmp_controller->walk_direction & WALK_LEFT)
    {
        glm_vec3_crossn(direction, cmp_camera->axisUp, cross);
        glm_vec3_scale(cross, speed, cross);

        glm_vec3_sub(newvel, cross, newvel);
    }
    if (cmp_controller->walk_direction & WALK_RIGHT)
    {
        glm_vec3_crossn(direction, cmp_camera->axisUp, cross);
        glm_vec3_scale(cross, speed, cross);

        glm_vec3_add(newvel, cross, newvel);
    }

#if 1
    // TODO: needs to be fixed by single-input check. Currently being called
    // several times.
    if (cmp_controller->is_grounded && cmp_controller->is_jumping)
    {
        cmp_controller->is_grounded = FALSE;
        cmp_controller->is_jumping  = FALSE;
        // cmp_rigidbody->linear_velocity[1] = newvel[1] + 5;

        // TODO: Better way to add forces
        cmp_rigidbody->force[1] =
          cmp_rigidbody->force[1] + cmp_controller->jump_force;
    }
#endif

    is_moving = (glm_vec3_norm(newvel) > 0);

    // directly modify velocity
    if (is_moving)
    {
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
                              .init         = (gsk_ECSSubscriber)init,
                              .update       = (gsk_ECSSubscriber)update,
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                            }));
}
