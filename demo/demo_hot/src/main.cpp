/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
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
    gsk::runtime::rt_setup(
      _PROJ_DIR_DATA, _PROJ_DIR_SCHEME, "GSK_Demo", argc, argv);

    demo_scenes_create(gsk::runtime::rt_get_ecs(),
                       gsk::runtime::rt_get_renderer(),
                       gsk::runtime::rt_get_asset_cache("demo://demo.test"));
    gsk::runtime::rt_set_scene(INITIAL_SCENE);

    gsk::runtime::rt_loop();

    return 0;
}
