/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __QMAP_BUILD_H__
#define __QMAP_BUILD_H__

#include "asset/qmap/qmapdefs.h"

#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void
gsk_qmap_build_polys_from_brush(gsk_QMapContainer *p_container,
                                gsk_QMapBrush *p_brush);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __QMAP_BUILD_H__