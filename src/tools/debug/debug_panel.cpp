#include "debug_panel.hpp"
#include <util/logger.h>

void
gsk::tools::DebugPanel::draw(void)
{
    LOG_INFO("Hello");
}

void
gsk::tools::DebugPanel::set_menu_index(int menu_index)
{
    this->menu_index = menu_index;
}