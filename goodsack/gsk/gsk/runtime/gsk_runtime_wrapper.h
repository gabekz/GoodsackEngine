#ifndef __GSK_RUNTIME_WRAPPER_H__
#define __GSK_RUNTIME_WRAPPER_H__

#include "asset/asset_cache.h"
#include "core/graphics/renderer/v1/renderer.h"
#include "entity/ecs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gsk_AssetCache *
gsk_runtime_get_asset_cache();

gsk_Renderer *
gsk_runtime_get_renderer();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_RUNTIME_WRAPPER_H__