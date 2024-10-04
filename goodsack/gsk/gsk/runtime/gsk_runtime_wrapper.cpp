#include "gsk_runtime.hpp"

extern "C" {
gsk_AssetCache *
gsk_runtime_get_asset_cache()
{
    return gsk::runtime::rt_get_asset_cache(); // Calls the C++ function
                                               // internally
}

gsk_Renderer *
gsk_runtime_get_renderer()
{
    return gsk::runtime::rt_get_renderer(); // Calls the C++ function
                                            // internally
}
}