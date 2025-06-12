/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "util/logger.h"

#include <gtest/gtest.h>

#define _USING_LISTENERS 0

#if _USING_LISTENERS
class PostTestLogicListener : public ::testing::EmptyTestEventListener {
    void OnTestEnd(const ::testing::TestInfo &info) override
    {
        const auto *p_result = info.result();

        std::cout << "[POST] " << info.test_suite_name()
                  << "."          // e.g. MathTests
                  << info.name(); // e.g. AddsTwoNumbers

        if (p_result->Passed())
            std::cout << " PASSED";
        else
            std::cout << " FAILED (" << p_result->Failed() << " assertions)";

        std::cout << std::endl;
    }
};
#endif // _USING_LISTENERS

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

#if _USING_LISTENERS
    // Add listeners
    ::testing::TestEventListeners &listeners =
      ::testing::UnitTest::GetInstance()->listeners();

    listeners.Append(new PostTestLogicListener);

    // NOTE: to remove GTest default console output:
    // listeners.Release(listeners.default_result_printer());
#endif // _USING_LISTENERS

    // Run tests
    return RUN_ALL_TESTS();
}