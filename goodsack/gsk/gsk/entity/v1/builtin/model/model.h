/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MODEL_C_H__
#define __MODEL_C_H__

// TODO: Remove file

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/mesh.h"
#include "core/graphics/mesh/model.h"

#include "entity/v1/ecs.h"

#define CULL_DISABLED 0x10
#define CULL_CW       0x00
#define CULL_CCW      0x01
#define CULL_FORWARD  0x00
#define CULL_BACK     0x02

#if !(USING_GENERATED_COMPONENTS)
struct ComponentModel
{
    Material *material;
    const char *modelPath;
    Mesh *mesh;
    Model *pModel;

    struct
    {
        ui16 renderMode : 1;
        ui16 drawMode : 2;
        ui16 cullMode : 3;
    } properties;

    ui32 vbo;
};
#endif

#endif // __MODEL_C_H__
