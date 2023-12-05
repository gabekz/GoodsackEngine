/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MESH_HELPERS_INL__
#define __MESH_HELPERS_INL__

#include <string.h>

#include "core/graphics/mesh/mesh.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

static void
_joint_transform_local(Joint *joint, float *outMatrix)
{
    float *lm = outMatrix;

    if (joint->pose.hasMatrix) {
        memcpy(lm, joint->pose.mTransform, sizeof(float) * 16);
    } else {
        Pose *node = &joint->pose;
        float tx   = node->translation[0];
        float ty   = node->translation[1];
        float tz   = node->translation[2];

        float qx = node->rotation[0];
        float qy = node->rotation[1];
        float qz = node->rotation[2];
        float qw = node->rotation[3];

        float sx = node->scale[0];
        float sy = node->scale[1];
        float sz = node->scale[2];

        lm[0] = (1 - 2 * qy * qy - 2 * qz * qz) * sx;
        lm[1] = (2 * qx * qy + 2 * qz * qw) * sx;
        lm[2] = (2 * qx * qz - 2 * qy * qw) * sx;
        lm[3] = 0.f;

        lm[4] = (2 * qx * qy - 2 * qz * qw) * sy;
        lm[5] = (1 - 2 * qx * qx - 2 * qz * qz) * sy;
        lm[6] = (2 * qy * qz + 2 * qx * qw) * sy;
        lm[7] = 0.f;

        lm[8]  = (2 * qx * qz + 2 * qy * qw) * sz;
        lm[9]  = (2 * qy * qz - 2 * qx * qw) * sz;
        lm[10] = (1 - 2 * qx * qx - 2 * qy * qy) * sz;
        lm[11] = 0.f;

        lm[12] = tx;
        lm[13] = ty;
        lm[14] = tz;
        lm[15] = 1.f;
    }
}

static void
_joint_transform_world(Joint *joint, float *outMatrix)
{
    float *lm = outMatrix;
    _joint_transform_local(joint, lm);

    Joint *parent = joint->parent;

    while (parent) {
        float pm[16];
        _joint_transform_local(parent, pm);

        for (int i = 0; i < 4; ++i) {

            float l0 = lm[i * 4 + 0];
            float l1 = lm[i * 4 + 1];
            float l2 = lm[i * 4 + 2];

            float r0 = l0 * pm[0] + l1 * pm[4] + l2 * pm[8];
            float r1 = l0 * pm[1] + l1 * pm[5] + l2 * pm[9];
            float r2 = l0 * pm[2] + l1 * pm[6] + l2 * pm[10];

            lm[i * 4 + 0] = r0;
            lm[i * 4 + 1] = r1;
            lm[i * 4 + 2] = r2;
        }

        lm[12] += pm[12];
        lm[13] += pm[13];
        lm[14] += pm[14];

        parent = parent->parent;
    }
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MESH_HELPERS_INL__
