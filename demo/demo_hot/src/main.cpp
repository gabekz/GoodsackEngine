/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifdef _WIN32
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#pragma optimize("", off)
#endif

#include <iostream>
#include <string>

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "scenes/demo_scenes.h"
#include <GoodsackEngineConfig.h> // TODO: Change this

#include "runtime/gsk_runtime.hpp"

int
main(int argc, char *argv[])
{
#if 0
    (gsk_ProjectInfo) {
      .name     = "Demo",
      .dir_root = _PROJ_DIR_ROOT,
    };
#endif

    // TODO: This is lazy
#ifdef SYS_ENV_WIN
    STATIC_ASSERT((_PROJ_DIR_ROOT), "project path-root definition required");
    STATIC_ASSERT((_GOODSACK_FS_DIR_ROOT), "GSK path-root definition required");
#endif
    gsk_runtime_setup((_PROJ_DIR_DATA "/"), _PROJ_DIR_SCHEME, argc, argv);

    demo_scenes_create(gsk_runtime_get_ecs(), gsk_runtime_get_renderer());
    gsk_runtime_set_scene(INITIAL_SCENE);

    gsk_runtime_loop();

    return 0;
}
