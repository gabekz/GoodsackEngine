/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __QMAP_MODEL_H__
#define __QMAP_MODEL_H__

#include "asset/qmap/qmapdefs.h"

#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture_set.h"

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_Model *
gsk_qmap_load_model(gsk_QMapContainer *p_container,
                    gsk_ShaderProgram *p_shader);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __QMAP_MODEL_H__