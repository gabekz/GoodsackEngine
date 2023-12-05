/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __SCENE_H__
#define __SCENE_H__

/* scene steps::
 * 1) Initialize and set active camera
 * 2) Create Light information
 [Standard]
 * 3) Renderer tick [all logic updates + shader updates]
 * 4) Render [all meshes in meshList]
 [ECS]
 * 3) Process Systems
 */

#include "util/sysdefs.h"
#include "core/graphics/lighting/skybox.h"

typedef struct gsk_Scene
{
    ui32 id, meshC, lightC;

    struct gsk_ECS *ecs;

    gsk_Skybox *skybox;
    ui16 has_skybox;
} gsk_Scene;

#endif // __SCENE_H__
