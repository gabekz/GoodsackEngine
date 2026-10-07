/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "console/console_cmd.h"
#include "console/console_var.h"

#include "util/logger.h"
#include "util/sysdefs.h"

// TODO: replace with thirdparty root-include
#include <gtest/gtest.h>

static void
cmd_echo(u32 argc, const char **argv)
{
    for (u32 i = 0; i < argc; i++)
    {
        LOG_PRINT("%s", argv[i]);
        if (i + 1 < argc) { LOG_PRINT(" "); }
    }
}

TEST(Console_Command, Functionality)
{
    gsk_console_cmd_register("echo", "echo <text..>", cmd_echo);

    u8 result = gsk_console_cmd_execute("echo 1 3 5");
    result    = gsk_console_cmd_execute("echo go fuck yourself");

    EXPECT_EQ(result, TRUE);

    gsk_console_var_register_int("testint", "this is a test", 32, 0, 64);
    gsk_console_var_print("testint");

    u8 status = FALSE;

    status = gsk_console_var_set_from_string("testint", "69");
    EXPECT_EQ(status, TRUE);
    gsk_console_var_print("testint");

    s32 a = gsk_console_var_get_int("testint", 0);
    EXPECT_EQ(64, a);

    gsk_console_var_register_string("teststr", "this is a test", "value");

    status = gsk_console_var_set_from_string("teststr", "hello world!!");
    EXPECT_EQ(status, TRUE);

    const char *p_str = gsk_console_var_get_string("teststr", "none");
    EXPECT_STREQ("hello world!!", p_str);
}