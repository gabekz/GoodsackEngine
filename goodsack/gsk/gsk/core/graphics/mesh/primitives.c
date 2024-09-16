/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "primitives.h"

#include <stdlib.h>

#include "util/sysdefs.h"

gsk_Mesh *
primitive_mesh_create(u32 shape, float scale)
{

    gsk_Mesh *ret = malloc(sizeof(gsk_Mesh));

    switch (shape)
    {
    case PRIMITIVE_CUBE:
        ret->meshData->vertexCount  = PRIM_SIZ_V_CUBE;
        ret->meshData->indicesCount = PRIM_SIZ_I_CUBE;
        break;
    }

    return ret;
}

//  * ----------- Rect -------------* //
float *
prim_vert_rect()
{
    unsigned int size = PRIM_SIZ_V_PLANE;
    float *ret        = malloc(size * sizeof(float));
    for (int i = 0; i < size; i++)
    {
        ret[i] = PRIM_ARR_V_PLANE[i];
    }
    return ret;
}
