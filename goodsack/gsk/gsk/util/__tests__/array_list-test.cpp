/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#include "util/array_list.h"
#include "util/sysdefs.h"

// TODO: replace with thirdparty root-include
#include <gtest/gtest.h>

TEST(Util_Array_list, Teardown)
{
    const u32 data_size = sizeof(u32);
    const u32 inc_size  = 10;
    ArrayList array     = array_list_init(sizeof(u32), inc_size);

    EXPECT_EQ(array.data.data_size, data_size);
    EXPECT_EQ(array.data.buffer_size, inc_size * data_size);

    EXPECT_EQ(array.is_list_empty, 1);
    EXPECT_EQ(array.list_next, 0);
    EXPECT_EQ(array.list_count, inc_size);
    EXPECT_EQ(array.list_increment, inc_size);

    u32 start_val  = 8;
    s32 iterations = 50;

    for (s32 i = 0; i < iterations; i++)
    {
        u32 data = start_val + i;
        array_list_push(&array, &data);
    }

    for (s32 i = 0; i < iterations; i++)
    {
        u32 val1 = *(u32 *)array_list_get_at_index(&array, i);
        u32 val2 = *(u32 *)array.data.buffer + i;

        EXPECT_EQ(val1, val2);

        EXPECT_EQ(val1, start_val + i);
        EXPECT_EQ(val2, start_val + i);
    }

    u32 new_expected_buffer_size =
      (inc_size * data_size) * (iterations / inc_size);

    EXPECT_EQ(array.data.data_size, data_size);
    EXPECT_EQ(array.data.buffer_size, new_expected_buffer_size);

    // popping
    EXPECT_EQ(array.list_next, iterations);
    array_list_pop(&array);
    EXPECT_EQ(array.list_next, iterations - 1);

#if 0
    for (s32 i = 0; i < iterations - 1; i++)
    {
        array_list_pop(&array);
    }

    array_list_pop(&array);
#else
    for (s32 i = 0; i < iterations; i++)
    {
        array_list_pop(&array);
    }
#endif

    EXPECT_EQ(array.list_next, 0);
    EXPECT_EQ(array.is_list_empty, 1);

    // disabled for now
    // EXPECT_EQ(array.data.buffer_size, inc_size * data_size);
}