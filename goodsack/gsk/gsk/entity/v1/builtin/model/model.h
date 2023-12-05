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
    gsk_Material *material;
    const char *modelPath;
    gsk_Mesh *mesh;
    gsk_Model *pModel;

    struct
    {
        u16 renderMode : 1;
        u16 drawMode : 2;
        u16 cullMode : 3;
    } properties;

    u32 vbo;
};
#endif

#endif // __MODEL_C_H__
