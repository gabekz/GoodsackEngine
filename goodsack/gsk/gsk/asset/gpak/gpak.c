/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gpak.h"

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/sysdefs.h"

#include "asset/asset_cache.h"

void
gsk_gpak_make_raw(gsk_AssetCache *p_cache)
{

    // get raw texture data
    // gsk_Texture tex = gsk_asset_cache_get_by_handle(p_cache, HANDLE);

    // 1. go through each texture asset
    // 2. get the handle and URI
    // 3. from the URI, get the image path
    // 4. get the raw image data
    // 5. store to gpak

    // need to store URI, then store binary location based on it
}