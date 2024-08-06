/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEMO_SCENES_H__
#define __DEMO_SCENES_H__

#include "core/graphics/renderer/v1/renderer.h"
#include "entity/ecs.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCENE_EARTH          0
#define SCENE_BOX            1
#define SCENE_PBR_SPHERES    2
#define SCENE_CERBERUS       3
#define SCENE_ANIMATOR       4
#define SCENE_SPONZA         5
#define SCENE_PHYSICS        6
#define SCENE_TRANSFORM_TEST 7
#define SCENE_MAP_TEST       8
#define DEMO_SCENES_TOTAL    SCENE_MAP_TEST

#define LOAD_ALL_SCENES 0
#define INITIAL_SCENE   SCENE_MAP_TEST

#define DEMO_USING_AUDIO            0
#define DEMO_USING_MULTIPLE_CAMERAS 1

#define TEX_OPS_NRM \
    (TextureOptions) { 1, GL_RGB, TRUE, TRUE }
#define TEX_OPS_PBR \
    (TextureOptions) { 8, GL_SRGB_ALPHA, TRUE, TRUE }

#define texture_create_d(x) texture_create(x, NULL, TEX_OPS_PBR)
#define texture_create_n(x) texture_create(x, NULL, TEX_OPS_NRM)

#define GRAVITY_EARTH      \
    {                      \
        0.0f, -9.81f, 0.0f \
    }

// Creates and loads every demo scene in this project.
void
demo_scenes_create(gsk_ECS *ecs, gsk_Renderer *renderer);

inline void
__set_active_scene_skybox(gsk_Renderer *renderer, gsk_Skybox *skybox)
{
    gsk_Scene *scene = renderer->sceneL[renderer->activeScene];

    scene->skybox     = skybox;
    scene->has_skybox = 1;
}

// outside scene declarations
void
_scene7(gsk_ECS *ecs, gsk_Renderer *renderer);

// outside scene declarations
void
_scene8(gsk_ECS *ecs, gsk_Renderer *renderer);

#ifdef __cplusplus
}
#endif

#endif // __DEMO_SCENES_H__
