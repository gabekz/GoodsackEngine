/*
 * Copyright (c) 2025-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gsk_runtime.hpp"

extern "C" {

gsk_ECS *
gsk_runtime_get_ecs()
{
    return gsk::runtime::rt_get_ecs(); // Calls the C++ function
                                       // internally
}

gsk_Renderer *
gsk_runtime_get_renderer()
{
    return gsk::runtime::rt_get_renderer(); // Calls the C++ function
                                            // internally
}

gsk_AssetCache *
gsk_runtime_get_asset_cache(const char *uri_str)
{
    return gsk::runtime::rt_get_asset_cache(uri_str); // Calls the C++ function
                                                      // internally
}

gsk_AssetRef *
gsk_runtime_get_fallback_asset(GskAssetType type)
{
    return gsk::runtime::rt_get_fallback_asset(type); // Calls the C++ function
                                                      // internally
}

char *
gsk_runtime_get_startup_map()
{
    return gsk::runtime::rt_get_startup_map(); // Calls the C++ function
                                               // internally
}

#if 0
gsk_Entity
gsk_runtime_get_debug_entity()
{
    return gsk::runtime::rt_get_debug_entity(); // Calls the C++ function
                                                // internally
}
#endif

gsk_EntityId
gsk_runtime_get_hovered_entity_id()
{
    return gsk::runtime::rt_get_hovered_entity_id(); // Calls the C++
                                                     // function internally
}

void
gsk_runtime_set_debug_entity_id(gsk_EntityId entity_id)
{
    return gsk::runtime::rt_set_debug_entity_id(
      entity_id); // Calls the C++
                  // function internally
}
}