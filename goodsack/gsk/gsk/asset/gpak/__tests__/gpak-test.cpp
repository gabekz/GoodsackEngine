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
    gsk_GPAK pak = gsk_gpak_init();
    gsk_gpak_write(&pak, "gsk://test/testfilefunctionhere.gltf", 32);

    u64 out = gsk_gpak_read(&pak, "gsk://test/testfilefunctionhere.gltf");
    EXPECT_EQ(out, 32);
}
