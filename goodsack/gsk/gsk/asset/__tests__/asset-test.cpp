/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "asset/asset_cache.h"

#include <gtest/gtest.h>

TEST(AssetTestSuite, FirstTest)
{
    gsk_AssetCache cache = gsk_asset_cache_init();

    gsk_asset_cache_add(&cache, 0, "gsk://textures/texture0.png");

    EXPECT_EQ(hash_table_get(&cache.asset_table, "gsk://textures/texture0.png"),
              1);
}
