#include <gtest/gtest.h>
#include <stdlib.h>

#include <util/filesystem.h>

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

    gsk_Path p1 = gsk_filesystem_path_from_uri("res://textures/white.jpg");
    ASSERT_STREQ(p1.path, "hello!");
}
