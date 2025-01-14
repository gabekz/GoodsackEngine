/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_panel.hpp"

#include "util/logger.h"

void
gsk::tools::DebugPanel::draw(void)
{
    LOG_WARN("unimplemented draw()");
}

void
gsk::tools::DebugPanel::set_menu_index(int menu_index)
{
    this->menu_index = menu_index;
}