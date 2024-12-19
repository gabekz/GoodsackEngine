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
}