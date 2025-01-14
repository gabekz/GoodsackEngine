/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __SERIALIZE_MODEL_H__
#define __SERIALIZE_MODEL_H__

#include "asset/assetdefs.h"
#include "core/graphics/mesh/model.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_AssetBlob
serialize_model(gsk_Model *p_model);

gsk_Model
extract_model(gsk_AssetBlob *p_blob);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SERIALIZE_MODEL_H__