/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "collider_setup-system.h"

#include <stdlib.h>

#include "util/maths.h"
#include "util/sysdefs.h"

#include "entity/ecs.h"

#include "core/device/device.h"
#include "core/graphics/mesh/mesh.h"

#include "physics/physics_collision.h"
#include "physics/physics_solver.h"

#include "runtime/gsk_runtime_wrapper.h"

#define MAX_COLLISION_POINTS         128
#define COLLISION_REQUIRES_RIGIDBODY FALSE

static void
init(gsk_Entity e)
{
    if (!(gsk_ecs_has(e, C_COLLIDER))) return;
    if (!(gsk_ecs_has(e, C_TRANSFORM))) return;

    struct ComponentCollider *collider   = gsk_ecs_get(e, C_COLLIDER);
    struct ComponentTransform *transform = gsk_ecs_get(e, C_TRANSFORM);

    collider->isColliding = FALSE; // TODO: remove <- this is for testing

    collider->pCollider = (gsk_Collider *)malloc(sizeof(gsk_Collider));
    ((gsk_Collider *)collider->pCollider)->collider_data_type = collider->type;

    //(gsk_Collider *)(collider->pCollider).position = transform->position;

    // TODO: collider types
    if (collider->type == COLLIDER_SPHERE)
    {
        gsk_SphereCollider *sphereCollider = malloc(sizeof(gsk_SphereCollider));

        if (collider->radius <= 0)
        {
            // default radius
            collider->radius = 0.5f;
        }

        sphereCollider->radius = collider->radius;

#if 0
        if (gsk_ecs_has(e, C_MODEL))
        {
            struct ComponentModel *cmp_model = gsk_ecs_get(e, C_MODEL);
            gsk_MeshData *meshdata = ((gsk_Mesh *)cmp_model->mesh)->meshData;

            f32 dist = glm_aabb_radius(meshdata->boundingBox);
            LOG_INFO("RADIUS IS %f", dist);
            sphereCollider->radius = dist / 2;
#if 0
            sphereCollider->radius =
              (dist / 2) * (glm_vec3_norm(transform->scale) / 2);
#endif
        }
#endif

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_SphereCollider *)sphereCollider;
    }

    else if (collider->type == COLLIDER_PLANE)
    {
        gsk_PlaneCollider *planeCollider = malloc(sizeof(gsk_PlaneCollider));

        // planeCollider->distance      = 0.0175f;
        planeCollider->distance = 10;
        glm_vec3_zero(planeCollider->plane);

        // TODO: Calculate normal from plane mesh with orientation
        vec3 planenorm = {0.0f, 1.0f, 0.0f};
        glm_vec3_copy(planenorm, planeCollider->normal);

        glm_vec3_copy(transform->position, planeCollider->plane);

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_PlaneCollider *)planeCollider;

#if 0
        LOG_INFO("%f\t%f\t%f",
                 planeCollider->plane[0],
                 planeCollider->plane[1],
                 planeCollider->plane[2]);
        // planeCollider->plane         = transform->position;

        // glm_vec3_normalize_to(planeCollider->plane, planeCollider->normal);

        LOG_INFO("%f\t%f\t%f",
                 planeCollider->normal[0],
                 planeCollider->normal[1],
                 planeCollider->normal[2]);
#endif
    }

    else if (collider->type == COLLIDER_BOX)
    {
        gsk_BoxCollider *box_collider = malloc(sizeof(gsk_BoxCollider));

        glm_vec3_zero(box_collider->bounds[0]);
        glm_vec3_zero(box_collider->bounds[1]);
        glm_vec3_zero(box_collider->center);

        if (gsk_ecs_has(e, C_MODEL) && collider->p_mesh != 0x32)
        {
            struct ComponentModel *cmp_model = gsk_ecs_get(e, C_MODEL);
            gsk_MeshData *meshdata = ((gsk_Mesh *)cmp_model->mesh)->meshData;
            glm_vec3_copy(meshdata->boundingBox[0], box_collider->bounds[0]);
            glm_vec3_copy(meshdata->boundingBox[1], box_collider->bounds[1]);

// TODO: Might want to get AABB center anyway
// either way, need to add "Center" for the box collider to offset from the
// position
#if 0
            glm_vec3_add(box_collider->bounds[0],
                         (vec3) {0, 1, 0},
                         box_collider->bounds[0]);
            glm_vec3_add(box_collider->bounds[1],
                         (vec3) {0, 1, 0},
                         box_collider->bounds[1]);
#endif

        }
        // TODO: maybe pass in the center as the body position for the friction
        // solver or solver_data?
        // TODO: TESTING
        else if (collider->p_mesh == 0x32)
        {
            glm_vec3_copy(collider->box_bounds_min, box_collider->bounds[0]);
            glm_vec3_copy(collider->box_bounds_max, box_collider->bounds[1]);

        }
        // TODO: TESTING
        else if (collider->p_mesh != NULL)
        {
            gsk_MeshData *meshdata = ((gsk_Mesh *)collider->p_mesh)->meshData;
            glm_vec3_copy(meshdata->boundingBox[0], box_collider->bounds[0]);
            glm_vec3_copy(meshdata->boundingBox[1], box_collider->bounds[1]);

        }
        // default BOX bounds
        else
        {
            LOG_WARN("no mesh found found box collider on entity %d. Setting "
                     "default bounds",
                     e.id);

            vec3 bounds_min = {-0.5f, -0.5f, -0.5f};
            vec3 bounds_max = {0.5f, 0.5f, 0.5f};
            glm_vec3_copy(bounds_min, box_collider->bounds[0]);
            glm_vec3_copy(bounds_max, box_collider->bounds[1]);
        }

#if 1
        glm_vec3_mul(
          box_collider->bounds[0], transform->scale, box_collider->bounds[0]);
        glm_vec3_mul(
          box_collider->bounds[1], transform->scale, box_collider->bounds[1]);
#endif

        glm_vec3_copy(collider->center, box_collider->center);

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_BoxCollider *)box_collider;
    }

    else if (collider->type == COLLIDER_CAPSULE)
    {
        gsk_CapsuleCollider *capsule_collider =
          malloc(sizeof(gsk_CapsuleCollider));

        vec3 base  = {0.0f, 1.255f, 0.0f};
        vec3 tip   = {0.0f, 0.5f, 0.0f};
        f32 radius = 0.2f;

// TODO: CHANGE THIS - temp for secondary capsule test
// TODO: TODAY
#if 1
        if (e.id >= 304)
        {
            vec3 new_base = {0.0f, -0.2f, 0.0f};
            vec3 new_tip  = {0.0f, 1.5f, 0.0f};
            glm_vec3_copy(new_base, base);
            glm_vec3_copy(new_tip, tip);
        }
#endif

        glm_vec3_copy(base, capsule_collider->base);
        glm_vec3_copy(tip, capsule_collider->tip);
        capsule_collider->radius = radius;

        ((gsk_Collider *)collider->pCollider)->collider_data =
          (gsk_CapsuleCollider *)capsule_collider;
    }
}

static void
destroy(gsk_Entity entity)
{
    if (!(gsk_ecs_has(entity, C_COLLIDER))) return;
    struct ComponentCollider *collider = gsk_ecs_get(entity, C_COLLIDER);

    if (collider->pCollider == NULL) { return; }
    gsk_Collider *p_col = (gsk_Collider *)collider->pCollider;

    if (p_col->collider_data) { free(p_col->collider_data); }

    free(p_col);
}

void
s_collider_setup_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init    = (gsk_ECSSubscriber)init,
                              .destroy = (gsk_ECSSubscriber)destroy,
                            }));
}
