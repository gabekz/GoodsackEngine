/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __QMAP_UTIL_H__
#define __QMAP_UTIL_H__

#include "asset/qmap/qmapdefs.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

u8
gsk_qmap_util_get_intersection(
  f32 *n1, f32 *n2, f32 *n3, f32 d1, f32 d2, f32 d3, f32 *output);

u8
gsk_qmap_util_compare_verts(const f32 *v1, const f32 *v2, const f32 epsilon);

void
gsk_qmap_util_plane_from_points(
  vec3 p1, vec3 p2, vec3 p3, f32 *norm_out, f32 *deter_out);

s32
gsk_qmap_util_classify_point(vec3 point, vec3 plane_norm, f32 plane_deter);

gsk_QMapEntityField *
gsk_qmap_util_get_field(gsk_QMapEntity *p_entity, const char *key_string);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __QMAP_UTIL_H__