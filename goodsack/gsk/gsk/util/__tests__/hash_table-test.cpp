/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "util/hash_table.h"
#include "util/sysdefs.h"

#include <gtest/gtest.h>

TEST(HashTableSuite, AddGet)
{
    HashTable table = hash_table_init(1000);
    hash_table_add(&table, "gsk://test/testfilefunctionhere.gltf", 32);

    u64 out = hash_table_get(&table, "gsk://test/testfilefunctionhere.gltf");
    EXPECT_EQ(out, 32);
}

TEST(HashTableSuite, Chaining_Single)
{
    // NOTE: effectively a linked-list

    HashTable table = hash_table_init(1);

    EXPECT_EQ(table.total_attribs, 1);

    hash_table_add(&table, "gsk://test/test1.gltf", 32);
    hash_table_add(&table, "gsk://test/test2.gltf", 64);
    hash_table_add(&table, "gsk://test/test3.gltf", 128);
    hash_table_add(&table, "gsk://test/test4.gltf", 256);
    hash_table_add(&table, "gsk://test/test5.gltf", 512);

    u64 out = hash_table_get(&table, "gsk://test/test1.gltf");
    EXPECT_EQ(out, 32);

    out = hash_table_get(&table, "gsk://test/test2.gltf");
    EXPECT_EQ(out, 64);

    out = hash_table_get(&table, "gsk://test/test3.gltf");
    EXPECT_EQ(out, 128);

    out = hash_table_get(&table, "gsk://test/test4.gltf");
    EXPECT_EQ(out, 256);

    out = hash_table_get(&table, "gsk://test/test5.gltf");
    EXPECT_EQ(out, 512);
}