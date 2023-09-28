#include "debug_panel.hpp"

#include <util/logger.h>

void
gsk::tools::DebugPanel::draw()
{
    LOG_INFO("Hello");
}

// ------- panels -------- //

void
gsk::tools::panels::SceneViewer::draw() {
    LOG_INFO("Overriden");
}
