/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gsk_generated/GoodsackEngineConfig.h"

#include "entity/component/ecs_component_layout_loader.hpp"
#include "util/logger.h"

int
main(int argc, char **argv)
{
    int logStat = logger_initConsoleLogger(NULL);

    logger_setLevel(LogLevel_TRACE);
    logger_setDetail(LogDetail_SIMPLE);

    if (logStat == 0) { exit(1); }

    LOG_INFO("Initialized Console Logger");
    LOG_INFO("Begin GSK_GEN Application");

    // Generate ECS Types
    {
        using namespace entity::component;

        ComponentLayoutMap map;

        parse_components_from_json(map,
                                   _GOODSACK_FS_DIR_DATA "/components.json");

        // TODO: parse_components for arg path passed in

        bool result =
          generate_cpp_types(_GOODSACK_FS_DIR_GEN "/ecs_components_gen.h", map);

        if (!result) { LOG_ERROR("Failed to generate ECS types!"); }
    }

    return 0;
}