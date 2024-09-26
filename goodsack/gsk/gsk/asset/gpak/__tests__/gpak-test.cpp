/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "asset/gpak/gpak.h"
#include "util/sysdefs.h"

#include <gtest/gtest.h>

TEST(GpakTestSuite, ReadWrite)
{
    gsk_GPAK pak = gsk_gpak_init(1000);
    gsk_gpak_write(&pak, "gsk://test/testfilefunctionhere.gltf", 32);

    u64 out = gsk_gpak_read(&pak, "gsk://test/testfilefunctionhere.gltf");
    EXPECT_EQ(out, 32);
}

TEST(GpakTestSuite, Chaining_Single)
{
    gsk_GPAK pak = gsk_gpak_init(1);

    EXPECT_EQ(pak.refs_table_count, 1);

    gsk_gpak_write(&pak, "gsk://test/test1.gltf", 32);
    gsk_gpak_write(&pak, "gsk://test/test2.gltf", 64);
    gsk_gpak_write(&pak, "gsk://test/test3.gltf", 128);
    gsk_gpak_write(&pak, "gsk://test/test4.gltf", 256);
    gsk_gpak_write(&pak, "gsk://test/test5.gltf", 512);

    u64 out = gsk_gpak_read(&pak, "gsk://test/test1.gltf");
    EXPECT_EQ(out, 32);

    out = gsk_gpak_read(&pak, "gsk://test/test2.gltf");
    EXPECT_EQ(out, 64);

    out = gsk_gpak_read(&pak, "gsk://test/test3.gltf");
    EXPECT_EQ(out, 128);

    out = gsk_gpak_read(&pak, "gsk://test/test4.gltf");
    EXPECT_EQ(out, 256);

    out = gsk_gpak_read(&pak, "gsk://test/test5.gltf");
    EXPECT_EQ(out, 512);
}