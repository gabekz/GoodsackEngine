/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "timer.h"

#include <stdio.h>

#include "util/sysdefs.h"

#ifdef SYS_ENV_WIN
#include <windows.h>
#else
#include <time.h>
#endif // SYS_ENV_WIN

f64
gsk_timer_get()
{
#ifdef _WIN32
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart / (double)f.QuadPart;
#else
    struct timespec t = {0};
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
#endif
}