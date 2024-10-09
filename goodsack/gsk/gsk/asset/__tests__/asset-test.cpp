/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "asset/asset_cache.h"
#include "asset/assetdefs.h"
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

    u32 expected_list  = 0; // table reference
    u32 expected_index = 1; // index reference
    u64 handle         = hash_table_get(&cache.asset_table, ASSET_NAME);

    EXPECT_EQ(GSK_ASSET_HANDLE_LIST_NUM(handle), expected_list);
    EXPECT_EQ(GSK_ASSET_HANDLE_INDEX_NUM(handle), expected_index);

    gsk_AssetRef *p_ref = gsk_asset_cache_get(&cache, ASSET_NAME);
    EXPECT_EQ(p_ref->is_imported, false);
    EXPECT_EQ(p_ref->is_utilized, false);
}