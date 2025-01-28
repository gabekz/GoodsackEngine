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

    gsk_ShaderProgram *cp_shader = gsk_shader_compute_program_create(
      GSK_PATH("zhr://shaders/fire_particles.compute"));

    gsk_ShaderProgram *dr_shader =
      GSK_ASSET("zhr://shaders/particles_computed.shader");

    gsk_Model *p_model_sphere = GSK_ASSET("gsk://models/suzanne.obj");

    // setup

    (gsk_ParticleSystem *)ent_emitter->p_particle_system =
      malloc(sizeof(gsk_ParticleSystem));

    *(gsk_ParticleSystem *)(ent_emitter->p_particle_system) =
      gsk_particle_system_create(
        cp_shader, dr_shader, p_model_sphere->meshes[0]->meshData);
}

static void
fixed_update(gsk_Entity ent)
{
    if (!(gsk_ecs_has(ent, C_PARTICLE_EMITTER))) return;
    if (!(gsk_ecs_has(ent, C_TRANSFORM))) return;

    struct ComponentParticleEmitter *ent_emitter =
      gsk_ecs_get(ent, C_PARTICLE_EMITTER);

    struct ComponentTransform *ent_transform = gsk_ecs_get(ent, C_TRANSFORM);

    gsk_ParticleSystem *p_sys =
      (gsk_ParticleSystem *)ent_emitter->p_particle_system;

    glm_vec3_copy(ent_transform->position, p_sys->world_pos);
    glm_vec3_copy(ent_transform->orientation, p_sys->world_rot);
    glm_vec3_copy(ent_transform->scale, p_sys->world_scale);
    gsk_particle_system_update(p_sys);
}

static void
render(gsk_Entity ent)
{
    if (ent.ecs->renderer->currentPass != SKYBOX_BEGIN) { return; }

    if (!(gsk_ecs_has(ent, C_PARTICLE_EMITTER))) return;
    struct ComponentParticleEmitter *ent_emitter =
      gsk_ecs_get(ent, C_PARTICLE_EMITTER);

    gsk_ParticleSystem *p_sys =
      (gsk_ParticleSystem *)ent_emitter->p_particle_system;

    gsk_particle_system_render(p_sys);
}

void
s_particles_ecs_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init         = (gsk_ECSSubscriber)init,
                              .fixed_update = (gsk_ECSSubscriber)fixed_update,
                              .render       = (gsk_ECSSubscriber)render,
                            }));
}
