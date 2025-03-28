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
#include "util/vec_colors.h"

#define DEFAULT_JUMP_FORCE 160

#define USING_COLLIDE_AND_SLIDE TRUE
#define COLLIDE_AND_SLIDE_DIST  1.4f
#define COLLIDE_AND_SLIDE_DEBUG FALSE

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
    cmp_controller->jump_force  = DEFAULT_JUMP_FORCE;

    cmp_controller->is_jumping = FALSE;
    cmp_controller->can_jump   = TRUE; // trigger that allows single jump input

    glm_vec2_zero(cmp_controller->move_axes);
}

static void
update(gsk_Entity entity)
{
    if (!gsk_ecs_has(entity, C_PLAYER_CONTROLLER)) { return; }

    struct ComponentPlayerController *cmp_controller =
      gsk_ecs_get(entity, C_PLAYER_CONTROLLER);

    // store *window for input
    GLFWwindow *window = entity.ecs->renderer->window;

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

    // zero-out walk direction, 2. check for input
    cmp_controller->walk_direction = 0;

    int input_up    = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    int input_down  = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    int input_left  = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    int input_right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

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

    // update move_axes
    cmp_controller->move_axes[0] = 0;
    cmp_controller->move_axes[1] = 0;
    if (cmp_controller->walk_direction & WALK_FORWARD)
    {
        cmp_controller->move_axes[1] = 1.0f;
    } else if (cmp_controller->walk_direction & WALK_BACKWARD)
    {
        cmp_controller->move_axes[1] = -1.0f;
    }

    if (cmp_controller->walk_direction & WALK_LEFT)
    {
        cmp_controller->move_axes[0] = -1.0f;
    } else if (cmp_controller->walk_direction & WALK_RIGHT)
    {
        cmp_controller->move_axes[0] = 1.0f;
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

    {
        gsk_Raycast raycast;
        vec3 dir = {0, -1.0f, 0};
        glm_vec3_copy(cmp_transform->position, raycast.origin);
        glm_vec3_copy(dir, raycast.direction);

        gsk_mod_RaycastResult result =
          gsk_mod_physics_raycast(entity, &raycast, 2.0f);

        cmp_controller->is_grounded = result.has_collision;
    }

    // --------- logic ---------

    vec3 direction = GLM_VEC3_ZERO_INIT, newvel = GLM_VEC3_ZERO_INIT;

    glm_vec3_copy(cmp_camera->front, direction);
    direction[1] = 0.0f;

    // normalize direction.xz
    {
        const f32 len = glm_vec3_norm(direction);
        if (len > 0.0f) { glm_vec3_scale(direction, 1.0f / len, direction); }
    }

    // scale by speed
    const f32 speed = cmp_controller->speed;
    glm_vec3_scale(direction, cmp_controller->speed, direction);

    f32 move_axes_norm = glm_vec2_norm(cmp_controller->move_axes);

    u8 is_gliding = (cmp_controller->is_grounded == FALSE &&
                     cmp_collider->isColliding == FALSE);

    // Calculate movement based on move_axes

    vec3 dir_horizontal, dir_vertical;
    glm_vec3_scale(direction, cmp_controller->move_axes[1], dir_vertical);
    glm_vec3_add(newvel, dir_vertical, newvel);

    f32 hor_speed = (is_gliding) ? speed * 0.25f : speed;

    glm_vec3_crossn(direction, cmp_camera->axisUp, dir_horizontal);
    glm_vec3_scale(
      dir_horizontal, hor_speed * cmp_controller->move_axes[0], dir_horizontal);
    glm_vec3_add(newvel, dir_horizontal, newvel);

    // set is_moving based on magnitude
    f32 newvel_mag = glm_vec3_norm(newvel);
    u8 is_moving   = (newvel_mag > 0.1f) ? 1 : 0;

#if 1
    // handle jump event (send force)
    if (cmp_controller->is_grounded && cmp_controller->is_jumping &&
        cmp_rigidbody->linear_velocity[1] <= 0.0f)
    {
        cmp_controller->is_grounded = FALSE;
        cmp_controller->is_jumping  = FALSE;
        // cmp_rigidbody->linear_velocity[1] = newvel[1] + 5;

        // TODO: Better way to add forces
        cmp_rigidbody->force_velocity[1] += cmp_controller->jump_force;
    }
#endif

    if (is_gliding)
    {
        vec3 jump_vel = GLM_VEC3_ZERO_INIT;
        glm_vec3_copy(newvel, jump_vel);

        if (move_axes_norm > 0)
        {
            cmp_rigidbody->linear_velocity[0] = jump_vel[0];
            cmp_rigidbody->linear_velocity[2] = jump_vel[2];
        }
    }

    // if (cmp_controller->is_grounded == FALSE &&
    //    cmp_collider->isColliding == FALSE)
    // if (is_moving == FALSE) { return; }
    // if (cmp_controller->is_grounded == FALSE) { return; }

    if (is_gliding || is_moving == FALSE ||
        cmp_controller->is_grounded == FALSE)
    {
        return;
    }

    // --------- Collide And Slide -----------------
#if USING_COLLIDE_AND_SLIDE
    {
        // gsk_Raycast raycast;
        // glm_vec3_copy(cmp_transform->position, raycast.origin);
        // glm_vec3_normalize_to(newvel, raycast.direction);

        vec3 offset = {0, 0, 0};
        glm_vec3_copy(cmp_transform->position, offset);
        offset[1] -= 0.2f;

        gsk_mod_RaycastResult result = gsk_mod_physics_capsuletest(
          entity, cmp_transform->position, newvel, COLLIDE_AND_SLIDE_DIST);

#if COLLIDE_AND_SLIDE_DEBUG
        gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                               MARKER_RAY,
                               327125,
                               offset,
                               newvel,
                               newvel_mag * 0.1f,
                               VCOL_BLUE,
                               FALSE);
#endif // COLLIDE_AND_SLIDE_DEBUG

        u8 can_slide = (result.has_collision == TRUE);
        if (can_slide)
        {
            if (gsk_ecs_has(result.entity, C_RIGIDBODY)) { can_slide = FALSE; }
        }

        if (can_slide)
        {
            f32 dist =
              glm_vec3_distance(cmp_transform->position, result.hit_position);

            vec3 snapped = GLM_VEC3_ZERO_INIT;
            glm_vec3_normalize_to(newvel, snapped);
            glm_vec3_scale(snapped, dist, snapped);

            // leftover = newvel - snapped
            vec3 leftover;
            glm_vec3_sub(newvel, snapped, leftover);

            // plane-projection
            float dotval = glm_vec3_dot(leftover, result.hit_normal);

            vec3 normalcomp;
            glm_vec3_scale(result.hit_normal, dotval, normalcomp);

            vec3 slide;
            glm_vec3_sub(leftover, normalcomp, slide);
            // glm_vec3_normalize(slide);
            // glm_vec3_scale(slide, glm_vec3_norm(newvel), slide);

            if (dist <= COLLIDE_AND_SLIDE_DIST)
            {
#if 0
                if (glm_vec3_norm(slide) <= 0.2f)
                {
                    glm_vec3_normalize(slide);
                    glm_vec3_scale(slide, 2.0f, slide);
                }
#endif

                // get dot for slide slowdown
                vec3 init_hor = {newvel[0], 0, newvel[2]};
                glm_vec3_normalize(init_hor);
                vec3 init_hit = {
                  -result.hit_normal[0], 0, -result.hit_normal[2]};
                f32 slide_scale = 1 - glm_dot(init_hor, init_hit);

                // get slide vec
                glm_vec3_normalize(slide);
                glm_vec3_scale(slide, newvel_mag, slide);
                glm_vec3_copy(slide, newvel);

                // update newvec
                glm_vec3_scale(newvel, slide_scale, newvel);
            }

#if COLLIDE_AND_SLIDE_DEBUG
            gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                                   MARKER_RAY,
                                   327128,
                                   result.hit_position,
                                   slide,
                                   newvel_mag * 0.1f,
                                   VCOL_RED,
                                   FALSE);

            gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                                   MARKER_POINT,
                                   327122,
                                   snapped,
                                   slide,
                                   5,
                                   VCOL_BLUE,
                                   FALSE);

            gsk_debug_markers_push(entity.ecs->renderer->debugContext,
                                   MARKER_POINT,
                                   327129,
                                   result.hit_position,
                                   slide,
                                   5,
                                   VCOL_GREEN,
                                   FALSE);
#endif // COLLIDE_AND_SLIDE_DEBUG
        }
    }
#endif // USING_COLLIDE_AND_SLIDE

    // --------- update Rigidbody velocity ---------

    cmp_rigidbody->linear_velocity[0] = newvel[0];
    cmp_rigidbody->linear_velocity[2] = newvel[2];
    // glm_vec3_copy(newvel, cmp_rigidbody->linear_velocity);
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
