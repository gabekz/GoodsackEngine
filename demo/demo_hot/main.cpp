#ifdef _WIN32
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#pragma optimize("", off)
#endif

#include <iostream>
#include <string>

#include <util/maths.h>
#include <util/sysdefs.h>

#include "demo_scenes.h"

#include <runtime/gsk_runtime.hpp>

int
main(int argc, char *argv[])
{
    gsk_runtime_setup(argc, argv);

    demo_scenes_create(gsk_runtime_get_ecs(), gsk_runtime_get_renderer());
    gsk_runtime_set_scene(5);

    gsk_runtime_loop();

    return 0;
}
