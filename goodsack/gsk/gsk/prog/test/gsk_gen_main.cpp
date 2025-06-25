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
    LOG_INFO("Begin Test Application");

    using namespace entity::component;

    ComponentLayoutMap map =
      parse_components_from_json(_GOODSACK_FS_DIR_DATA "/components.json", 0);

    generate_cpp_types(_GOODSACK_FS_DIR_GEN "/ecs_components_gen.h", map);

    return 0;
}