/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>

#include "util/logger.h"

int
main(int argc, char **argv)
{
    // initialize GTest
    ::testing::InitGoogleTest(&argc, argv);

    // initialize logger
    int logStat = logger_initConsoleLogger(NULL);
    // logger_initFileLogger("logs/logs.txt", 0, 0);

    logger_setLevel(LogLevel_TRACE);
    logger_setDetail(LogDetail_SIMPLE);

    if (logStat == 0) { exit(1); }

    LOG_INFO("Initialized Console Logger");
    LOG_INFO("Begin Test Application");

    // Run tests
    return RUN_ALL_TESTS();
}