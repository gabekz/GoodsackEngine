/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "primitives.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include <stdlib.h>

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

gsk_Mesh *
primitive_sphere_create(f32 radius, f32 sector_count, f32 stack_count)
{
    ArrayList list_verts = LIST_INIT(sizeof(f32), 10);

    // vertex
    // f32 x, y, z, xy;                            // position
    f32 nx, ny, nz, length_inv = 1.0f / radius; // normal
    f32 s, t;                                   // texture

    f32 sector_step = 2.0f * PI / sector_count;
    f32 stack_step  = PI / stack_count;
    f32 sector_angle, stack_angle;

    u32 n_verts = 0;

    for (int i = 0; i <= stack_count; i++)
    {
        stack_angle = PI / 2.0f - i * stack_step; // starting from pi/2 to -pi/2

        f32 xy = radius * cosf(stack_angle); // r * cos(u)
        f32 z  = radius * sinf(stack_angle); // r * sin(u)

        for (int j = 0; j <= sector_count; j++)
        {
            sector_angle = j * sector_step; // starting from 0 to 2pi

            // calculate vertex position
            f32 x = xy * cosf(sector_angle); // r * cos(u) * cos(v)
            f32 y = xy * sinf(sector_angle); // r * cos(u) * sin(v)
            // PUSH BACK xyz
            array_list_push(&list_verts, &x);
            array_list_push(&list_verts, &y);
            array_list_push(&list_verts, &z);

#if 0
            // calculate normalized vertex normal
            vec3 n = {x * length_inv, y * length_inv, z * length_inv};
            // PUSH BACK
            array_list_push(&list_verts, &n[0]);
            array_list_push(&list_verts, &n[1]);
            array_list_push(&list_verts, &n[2]);

            // tex coords
            vec2 t = {(f32)j / sector_count, (f32)i / stack_count};
            // PUSH BACK
            array_list_push(&list_verts, &t[0]);
            array_list_push(&list_verts, &t[1]);
#endif

            n_verts++;
        }
    }

    // CREATE NEW MESHDATA
    gsk_MeshData *meshdata = malloc(sizeof(gsk_MeshData));
    meshdata->usage_draw   = GskOglUsageType_Static;

    meshdata->mesh_buffers_count = 0;

    meshdata->mesh_buffers_count++;
    meshdata->mesh_buffers[0] = (gsk_MeshBuffer)
    {
#if 0
      .buffer_flags = (GskMeshBufferFlag_Positions |
                       GskMeshBufferFlag_Textures | GskMeshBufferFlag_Normals),
#else
        .buffer_flags = (GskMeshBufferFlag_Positions),
#endif
        .p_buffer    = list_verts.data.buffer,
        .buffer_size = list_verts.data.buffer_size,
    };

#if 0
    meshdata->mesh_buffers_count++;
    meshdata->mesh_buffers[1] = (gsk_MeshBuffer) {
      .buffer_flags = (GskMeshBufferFlag_Indices),
      .p_buffer     = buff_indices,
      .buffer_size  = num_indices * sizeof(u32),
    };
#endif

    meshdata->vertexCount    = n_verts;
    meshdata->indicesCount   = 0;
    meshdata->primitive_type = GskMeshPrimitiveType_Triangle;

    gsk_Mesh *ret = gsk_mesh_allocate(meshdata);
    gsk_mesh_assemble(ret);
    ret->usingImportedMaterial = FALSE;
    return ret;
}