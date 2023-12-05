/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef H_DEMO_SCENES
#define H_DEMO_SCENES

#include "core/graphics/renderer/v1/renderer.h"
#include "entity/v1/ecs.h"

#ifdef __cplusplus
extern "C" {
#endif

enum DEMO_SCENE_NAMES {
    kSceneEarth = 0,
    kSceneBox,
    kScenePbrSpheres,
    kSceneCerberus,
    kSceneAnimator,
    kSceneSponza,
    kScenePhysics,
    kSceneTransformTest,
};
#define DEMO_SCENES_TOTAL 7

#define LOAD_ALL_SCENES 0
#define INITIAL_SCENE   7

#define DEMO_USING_AUDIO            0
#define DEMO_USING_MULTIPLE_CAMERAS 0

// Creates and loads every demo scene in this project.
void
demo_scenes_create(ECS *ecs, gsk_Renderer *renderer);

#ifdef __cplusplus
}
#endif

#endif // H_DEMO_SCENES
