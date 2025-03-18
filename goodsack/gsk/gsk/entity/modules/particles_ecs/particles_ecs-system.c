/*
 * Copyright (c) 2025-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */
#include "particles_ecs-system.h"

#include "asset/asset.h"
#include "core/graphics/particles/particle_system.h"
#include "entity/ecs.h"

#include "util/filesystem.h"
#include "util/logger.h"

static void
init(gsk_Entity ent)
{
    if (!(gsk_ecs_has(ent, C_TRANSFORM))) return;
    if (!(gsk_ecs_has(ent, C_PARTICLE_EMITTER))) return;

    struct ComponentTransform *ent_transform = gsk_ecs_get(ent, C_TRANSFORM);
    struct ComponentParticleEmitter *ent_emitter =
      gsk_ecs_get(ent, C_PARTICLE_EMITTER);

    // gsk_particle_system_init(NULL, NULL, NULL);

#if 0
    gsk_ShaderProgram *cp_shader = gsk_shader_compute_program_create(
      GSK_PATH("zhr://shaders/fire_particles.compute"));

    gsk_ShaderProgram *dr_shader =
      GSK_ASSET("zhr://shaders/particles_computed.shader");
#endif

    gsk_MeshData *p_explicitdata = NULL;

#if 1
    if (ent_emitter->p_meshdata)
    {
        p_explicitdata = (gsk_MeshData *)ent_emitter->p_meshdata;
    }
#endif

    else if (gsk_ecs_has(ent, C_MODEL))
    {
        struct ComponentModel *ent_model = gsk_ecs_get(ent, C_MODEL);
        gsk_Model *p_model               = (gsk_Model *)(ent_model->pModel);
        p_explicitdata                   = p_model->meshes[0]->meshData;
    }
    // FALLBACK
    else
    {
        LOG_DEBUG("using fallback for particle emitter for entity %d", ent.id);

        gsk_Model *p_model_emitter = GSK_ASSET("gsk://models/cube.obj");
        p_explicitdata             = p_model_emitter->meshes[0]->meshData;
    }

    if (p_explicitdata == NULL)
    {
        LOG_CRITICAL("failed to get fallback particle emitter mesh");
    }

    // create new particle system

    gsk_ShaderProgram *p_shader_com =
      GSK_ASSET("zhr://shaders/fire_particles.compute");

    gsk_ShaderProgram *p_shader_ren =
      GSK_ASSET("zhr://shaders/particles_computed.shader");

    gsk_ParticleSystem *p_sys_new = malloc(sizeof(gsk_ParticleSystem));
    *p_sys_new =
      gsk_particle_system_create(p_shader_com, p_shader_ren, p_explicitdata);

    ent_emitter->p_particle_system = p_sys_new;
}

static void
fixed_update(gsk_Entity ent)
{
    if (!(gsk_ecs_has(ent, C_PARTICLE_EMITTER))) return;

    struct ComponentParticleEmitter *ent_emitter =
      gsk_ecs_get(ent, C_PARTICLE_EMITTER);

    // do nothing if emitter is not awake
    if (ent_emitter->is_awake == FALSE) { return; }

    if (!(gsk_ecs_has(ent, C_TRANSFORM))) return;
    struct ComponentTransform *ent_transform = gsk_ecs_get(ent, C_TRANSFORM);

    gsk_ParticleSystem *p_sys =
      (gsk_ParticleSystem *)ent_emitter->p_particle_system;

    glm_vec3_copy(ent_transform->world_position, p_sys->world_pos);
    glm_vec3_copy(ent_transform->orientation, p_sys->world_rot);
    glm_vec3_copy(ent_transform->scale, p_sys->world_scale);

#if 0
    if (gsk_ecs_has(ent, C_BONE_ATTACHMENT))
    {
        f32 oldz            = -p_sys->world_rot[2];
        p_sys->world_rot[2] = -p_sys->world_rot[1];
        p_sys->world_rot[1] = oldz;
    }
#endif

    // glm_vec3_scale(p_sys->world_scale, 1.005f, p_sys->world_scale);
    gsk_particle_system_update(p_sys);
}

static void
render(gsk_Entity ent)
{
    if (ent.ecs->renderer->currentPass != GskRenderPass_Skybox) { return; }

    if (!(gsk_ecs_has(ent, C_PARTICLE_EMITTER))) return;
    struct ComponentParticleEmitter *ent_emitter =
      gsk_ecs_get(ent, C_PARTICLE_EMITTER);

    // do nothing if emitter is not awake
    if (ent_emitter->is_awake == FALSE) { return; }

    gsk_ParticleSystem *p_sys =
      (gsk_ParticleSystem *)ent_emitter->p_particle_system;

    gsk_particle_system_render(p_sys);
}

static void
destroy(gsk_Entity ent)
{
    if (!(gsk_ecs_has(ent, C_PARTICLE_EMITTER))) return;

    struct ComponentParticleEmitter *ent_emitter =
      gsk_ecs_get(ent, C_PARTICLE_EMITTER);

    ent_emitter->is_awake = FALSE;

    gsk_ParticleSystem *p_sys =
      (gsk_ParticleSystem *)ent_emitter->p_particle_system;

    gsk_particle_system_cleanup(p_sys);
    p_sys = NULL;
}

void
s_particles_ecs_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init         = (gsk_ECSSubscriber)init,
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                              .render       = (gsk_ECSSubscriber)render,
                              .destroy      = (gsk_ECSSubscriber)destroy,
                            }));
}
