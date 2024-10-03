/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "asset/asset_cache.h"
#include "util/sysdefs.h"

// #include "core/graphics/texture/texture.h"

#include <gtest/gtest.h>

#define ASSET_NAME "gsk://textures/texture0.png"
#define ASSET_TYPE 0

TEST(AssetTestSuite, HashTest)
{
    gsk_AssetCache cache = gsk_asset_cache_init();

    gsk_asset_cache_add(&cache, ASSET_TYPE, ASSET_NAME);
    EXPECT_EQ(hash_table_has(&cache.asset_table, ASSET_NAME), true);

    u64 expected_handle = 1; // index reference
    EXPECT_EQ(hash_table_get(&cache.asset_table, ASSET_NAME), expected_handle);

    gsk_AssetCacheState *p_state =
      gsk_asset_cache_get(&cache, ASSET_TYPE, ASSET_NAME);
    EXPECT_EQ(p_state->is_loaded, false);
}