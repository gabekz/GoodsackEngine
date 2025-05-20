/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "util/filesystem.h"
#include "util/sysdefs.h"

#include <GoodsackEngineConfig.h>

// TODO: replace with thirdparty root-include
#include <gtest/gtest.h>

TEST(Util_Filesystem, URI_Parse)
{
    gsk_URI uri1 = gsk_filesystem_uri("res://textures/black.png");
    ASSERT_STREQ(uri1.scheme, "res");
    ASSERT_STREQ(uri1.macro, ":");
    ASSERT_STREQ(uri1.path, "textures/black.png");

    gsk_URI uri2 = gsk_filesystem_uri("data:shaders//shader.glsl");
    ASSERT_STREQ(uri2.scheme, "data");
    ASSERT_STREQ(uri2.macro, ":shaders");
    ASSERT_STREQ(uri2.path, "shader.glsl");

    // Incomplete/bad parse
    gsk_URI uri3 = gsk_filesystem_uri("data//shader.glsl");
    EXPECT_EQ(uri3.scheme[0], NULL);
    EXPECT_EQ(uri3.macro[0], NULL);
    EXPECT_EQ(uri3.path[0], NULL);
}

TEST(Util_Filesystem, Path_Checking)
{
    gsk_filesystem_initialize("test", "test2");

    gsk_Path p1 = gsk_filesystem_uri_to_path("gsk://textures/white.jpg");
    ASSERT_STREQ(p1.path, _GOODSACK_FS_DIR_DATA "/textures/white.jpg");

    char p2[GSK_FS_MAX_PATH];
    gsk_filesystem_path_to_uri(_GOODSACK_FS_DIR_DATA "/textures/black.png", p2);
    ASSERT_STREQ(p2, "gsk://textures/black.png");
}