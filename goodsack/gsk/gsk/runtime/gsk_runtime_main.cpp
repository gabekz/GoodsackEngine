/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "runtime/gsk_runtime.hpp"

#include <iostream>
#if defined (SYS_ENV_WIN)
#include <filesystem>
#endif

#include "util/sysdefs.h"

static void
_gsk_getFilepathInfo()
{
#if defined(SYS_ENV_WIN32)
    std::cout << "Current path is " << std::filesystem::current_path() << '\n';
    std::filesystem::current_path(fs::temp_directory_path());
    std::cout << "Current path is " << std::filesystem::current_path() << '\n';
#endif
}

int
main(int argc, char *argv[])
{
    gsk_runtime_setup(NULL, argc, argv);
    gsk_runtime_loop();

    _gsk_getFilepathInfo();
}
