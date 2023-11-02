#ifdef _WIN32
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#pragma optimize("", off)
#endif

#include <iostream>
#include <string>

#include <util/logger.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include "demo_scenes.h"

#include <runtime/gsk_runtime.hpp>

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
    STATIC_ASSERT((_PROJ_DIR_ROOT), "path-root definition required");
#endif
    gsk_runtime_setup((_PROJ_DIR_DATA "/"), argc, argv);

    demo_scenes_create(gsk_runtime_get_ecs(), gsk_runtime_get_renderer());
    gsk_runtime_set_scene(INITIAL_SCENE);

    gsk_runtime_loop();

    return 0;
}
