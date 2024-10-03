/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "asset/asset_cache.h"
#include "core/graphics/texture/texture.h"

#include <gtest/gtest.h>

TEST(AssetTestSuite, FirstTest)
{
    gsk_AssetCache cache = gsk_asset_cache_init();

    gsk_asset_cache_add(&cache, 0, "gsk://textures/texture0.png");
    EXPECT_EQ(hash_table_has(&cache.asset_table, "gsk://textures/texture0.png"),
              1);

    EXPECT_EQ(hash_table_get(&cache.asset_table, "gsk://textures/texture0.png"),
              1);

    gsk_AssetCacheState *p_state =
      gsk_asset_cache_get(&cache, 0, "gsk://textures/texture0.png");
    EXPECT_EQ(p_state->is_loaded, 0);
}

TEST(AssetTestSuite, QuickTextureTest)
{
    gsk_AssetCache cache = gsk_asset_cache_init();

    gsk_Texture texture = {
      .id = 69,
    };

    gsk_asset_cache_add(&cache, 0, "gsk://textures/texture0.png");

    EXPECT_EQ(hash_table_get(&cache.asset_table, "gsk://textures/texture0.png"),
              1);
}